/* server/Channels.cpp - Channels functions
 *
 * Copyright (C) 2005-2006 Romain Bignon  <Progs@headfucking.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#include "Channels.h"
#include "Server.h"
#include "Commands.h"
#include "Outils.h"
#include "Debug.h"
#include "Main.h"
#include "Map.h"
#include <cstdarg>

ChannelVector ChanList;

/********************************************************************************************
 *                              Commandes                                                   *
 ********************************************************************************************/

/** A player wants to send a message to channel.
 *
 * Syntax: MSG message
 */
int MSGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player())
		return vDebug(W_DESYNCH, "MSG en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	cl->Player()->Channel()->sendto_players(cl->Player(),
			app.rpl(ECServer::MSG), cl->GetNick(), FormatStr(parv[1].c_str()));
	return 0;
}

/** A player wants to set an attribute in channel.
 *
 * Syntax: SET modes [params ...]
 */
int SETCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || !cl->Player()->Channel())
		return Debug(W_DESYNCH, "SET en dehors d'un chan");

	ECPlayer *sender = cl->Player();

	bool add = true;
	bool ready = false;
	uint j = 2;
	struct
	{
		char c;
		bool changed;
		bool add;
		const char* params;
	} newmodes[] = {
		{ 'm', false, false, NULL},
		{ 'o', false, false, NULL},
		{ 'c', false, false, NULL},
		{ 'p', false, false, NULL},
		{ '!', false, false, NULL},
	};

	for(uint i=0; i<parv[1].size(); i++)
	{
		short changed = 0;
		const short YES_NOPARAMS = 1;
		const short YES_WITHPARAM = 2;
		switch(parv[1][i])
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'm':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cm: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsPriv())
					return Debug(W_DESYNCH, "SET %c%c: d'un non privilégié", add ? '+' : '-', parv[1][i]);

				if(add)
				{
					if(j<parv.size())
					{
						uint mapi = StrToTyp<uint>(parv[j++]);
						if(mapi >= MapList.size())
							Debug(W_DESYNCH, "SET +m: de la map %d hors de la liste (%d)", mapi, MapList.size());
						else
						{
							ECMap* map = MapList[mapi];
							if(map == sender->Channel()->Map())
								continue; /* anti-flood */
							sender->Channel()->SetMap(map);
							sender->Channel()->NeedReady();
							changed = YES_WITHPARAM;
						}
					}
				}
				else
					Debug(W_DESYNCH, "SET -m: interdit.");
				break;
			case '!':
				/* Autorise seulement à se déclarer comme OK, ne peut en aucun cas retirer ce qu'il
				 * a dit par la suite.
				 */
				if(!add)
					{ Debug(W_DESYNCH, "SET %c%c: interdit.", add ? '+' : '-', parv[1][i]); break; }

				if(sender->Ready())
				{
					Debug(W_WARNING, "SET +%c: sender->Ready()=TRUE", parv[1][i]);
					break;
				}
				if(!sender->Position() || !sender->Color())
				{ /* Pas besoin de vérifier si il y a une map ou si c'est en cours de partie, car dans
				   * ces deux cas le joueur a une position et une couleur !
				   */
					vDebug(W_DESYNCH, "SET +!: alors qu'il n'est pas pret",
					                  VIName(sender->Position()) VIName(sender->Color()));
					break;
				}
				ready = true;
				sender->SetReady(add);
				changed = YES_NOPARAMS;
				break;
			case 'o':
			{
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %co: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsOwner())
					{ Debug(W_DESYNCH, "SET %c%c: d'un non owner", add ? '+' : '-', parv[1][i]); break; }
				if(j>=parv.size()) { Debug(W_DESYNCH, "SET %co: pas de nick", add ? '+' : '-'); break; }
				ECPlayer *pl = sender->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH, "SET %co: %s non trouvé", add ? '+' : '-', parv[(j-1)].c_str()); break; }
				if(pl->IsOwner())
				{
					Debug(W_DESYNCH, "SET %co: %s est owner et ne peut pas être op.", parv[(j-1)].c_str());
					break;
				}
				pl->SetOp(add);
				changed = YES_WITHPARAM;
				break;
			}
			case 'c':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cc: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->Channel()->Map())
				{
					Debug(W_DESYNCH, "SET %cc: alors qu'il n'y a pas de map", add ? '+' : '-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +c: sans couleur"); break; }
					uint color = StrToTyp<uint>(parv[j++]);
					if(color > 0)
					{
						BPlayerVector::iterator it;
						for(it = sender->Channel()->Players().begin();
						    it != sender->Channel()->Players().end() && (*it)->Color() != color;
						    ++it);
						if(it != sender->Channel()->Players().end())
							{ Debug(W_DESYNCH, "SET +c: d'une couleur déjà utilisée"); break; }
					}
					sender->SetColor(color);
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetColor(0);
					changed = YES_NOPARAMS;
				}
				break;
			case 'p':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cp: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->Channel()->Map())
				{
					Debug(W_DESYNCH, "SET %cp: alors qu'il n'y a pas de map", add ? '+' : '-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +p: sans couleur"); break; }
					uint place = StrToTyp<uint>(parv[j++]);
					if(place > 0)
					{
						if(place > sender->Channel()->GetLimite())
						{
							Debug(W_DESYNCH, "SET +p %d > %d(limite)", place,
							                 sender->Channel()->GetLimite());
							break;
						}
						BPlayerVector::iterator it;
						for(it = sender->Channel()->Players().begin();
						    it != sender->Channel()->Players().end() && (*it)->Position() != place;
						    ++it);
						if(it != sender->Channel()->Players().end())
							{ Debug(W_DESYNCH, "SET +p: d'une position déjà utilisée"); break; }
					}
					sender->SetPosition(place);
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetPosition(0);
					changed = YES_NOPARAMS;
				}
				break;
			default:
				Debug(W_DESYNCH, "SET %c%c: Reception d'un mode non supporté", add ? '+' : '-', parv[1][i]);
				break;
		}
		if(changed)
		{
			for(uint k=0;k<ASIZE(newmodes);k++)
				if(parv[1][i] == newmodes[k].c)
				{
					newmodes[k].changed = true;
					newmodes[k].add = add;
					if(changed == YES_WITHPARAM)
						newmodes[k].params = parv[(j-1)].c_str();
					break;
				}
		}
	}

	std::string modes = "", params = "";
	for(uint k=0;k<ASIZE(newmodes);k++)
		if(newmodes[k].changed)
		{
			if(modes.empty() || newmodes[k].add != add)
				modes += newmodes[k].add ? '+' : '-',
				add = newmodes[k].add;

			modes += newmodes[k].c;
			if(newmodes[k].params) params += " " + std::string(newmodes[k].params);
		}

	if(!modes.empty())
		sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), cl->GetNick(),
		                                 (modes + params).c_str());

	/* Si tout le monde est READY, on passe d'un etat de la partie à un autre. */
	if(ready)
	{
		uint c = 0;
		BPlayerVector pv = sender->Channel()->Players();
		for(BPlayerVector::iterator it=pv.begin(); it != pv.end(); ++it)
			if((*it)->Ready())
				c++;
		if(c == sender->Channel()->NbPlayers() || c == sender->Channel()->GetLimite())
		{
			switch(sender->Channel()->State())
			{
				case EChannel::WAITING:
				{
					if(c < sender->Channel()->Map()->MinPlayers())
						break; /* Si c == nbplayers, c'est ok que si nbplayers >= minplayers */

					/* Si le salon est une pré-partie et qu'il y a eu un +!, on vérifie que
					* si tout le monde est READY pour, dans ce cas là, lancer la partie
					* et jarter ceux qui sont en trop.
					*/
					for(BPlayerVector::iterator it=pv.begin(); it != pv.end(); ++it)
						if(!(*it)->Ready())
						{
							TClient* cl = (dynamic_cast<ECPlayer*>(*it))->Client();
							sender->Channel()->sendto_players(0, app.rpl(ECServer::LEAVE), cl->GetNick());
		
							sender->Channel()->RemovePlayer(*it, true);
							if(!sender->Channel()->NbPlayers())
							{
								delete sender->Channel();
								Debug(W_ERR, "SET:%d: heuuuu, pourquoi on passe par là ?", __LINE__);
							}
							cl->ClrPlayer();
						}
					sender->Channel()->SetState(EChannel::SENDING);
					sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), app.ServerName(), "-W+S");
					sender->Channel()->NeedReady();
					app.NBwchan--;
					app.NBachan++;
					break;
				}
				case EChannel::SENDING:
				{
					/* Le client est pret (a tout affiché, mémorisé, ...), la partie se lance donc et on
					 * commence en PLAYING.
					 */
					sender->Channel()->SetState(EChannel::PLAYING);
					sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), app.ServerName(), "-S+P");
					sender->Channel()->NeedReady();
				}
				default: /** @todo Non supportés encore: +AP */
					break;
			}
		}
	}

	return 0;
}

/** A client wants to join a channel.
 *
 * Syntax: JOI nom
 */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* Ne peut être que sur un seul salon à la fois */
	if(cl->Player() || parv[1].empty())
		return vDebug(W_WARNING, "JOI: Essaye de joindre plusieurs salons", VSName(cl->GetNick())
		                          VSName(cl->Player()->Channel()->GetName()) VName(parv[1]));

	const char* nom = parv[1].c_str();
	EChannel* chan = NULL;
	ECPlayer* pl;

	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); ++it)
		if(!strcasecmp((*it)->GetName(), nom))
		{
			chan = *it;
			break;
		}

	if(!chan)
	{ /* Création du salon */
		chan = new EChannel(nom);
		pl = new ECPlayer(cl, chan, true, false);
		ChanList.push_back(chan);
	}
	else
	{ /* Rejoins un salon existant */
		if(!chan->Joinable())
		{
			vDebug(W_WARNING, "JOI: Le client essaye de joindre un salon en jeu", VSName(chan->GetName())
			                  VSName(cl->GetNick()) VIName(chan->State()));
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		}
		if(chan->GetLimite() && chan->NbPlayers() >= chan->GetLimite())
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		pl = new ECPlayer(cl, chan, false, false);
	}
	cl->SetPlayer(pl);

	/** @page JOIN_MSGS Join's messages
	 *
	 * Messages sent on a join are :
	 * <pre>
	 * :me JOI chan
	 * LSM map1 1 5
	 * LSM map2 1 5
	 * EOM
	 * :server SET +lWm
	 * SMAP ligne1
	 * SMAP ligne2
	 * EOSMAP
	 * PLIST 0,0,me \@1,5prout
	 * </pre>
	 * @date 17/02/2006
	 */

	chan->sendto_players(0, app.rpl(ECServer::JOIN), cl->GetNick(), FormatStr(chan->GetName()), "");

	for(MapVector::iterator it = MapList.begin(); it != MapList.end(); ++it)
		cl->sendrpl(app.rpl(ECServer::LISTMAP), FormatStr((*it)->Name().c_str()),
		                                        (*it)->MinPlayers(), (*it)->MaxPlayers());
	cl->sendrpl(app.rpl(ECServer::ENDOFMAP));

	cl->sendrpl(app.rpl(ECServer::SET), app.GetConf()->ServerName().c_str(), chan->ModesStr());

	if(chan->Map())
	{ /* Si la map existe on l'envoie */
		std::vector<std::string> map_file = chan->Map()->MapFile();
		for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
			cl->sendrpl(app.rpl(ECServer::SENDMAP), FormatStr((*it).c_str()));
		cl->sendrpl(app.rpl(ECServer::ENDOFSMAP));
	}
	
	cl->sendrpl(app.rpl(ECServer::PLIST), chan->PlayerList());

	return 0;
}

/** A player wants to leave a channel.
 *
 * Syntax: LEA
 */
int LEACommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* N'est pas dans un salon */
	if(!cl->Player())
		return vDebug(W_DESYNCH, "LEA en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	EChannel *chan = cl->Player()->Channel();
	chan->sendto_players(0, app.rpl(ECServer::LEAVE), cl->GetNick());

	chan->RemovePlayer(cl->Player(), true);
	if(!chan->NbPlayers())
		delete chan;

	cl->ClrPlayer();

	return 0;
}

/** A player wants to list all channels.
 *
 * Syntax: LSP
 */
int LSPCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); ++it)
		if((*it)->Joinable())
			cl->sendrpl(app.rpl(ECServer::GLIST), FormatStr((*it)->GetName()), (*it)->NbPlayers(),
			                                      (*it)->GetLimite());

	return cl->sendrpl(app.rpl(ECServer::EOGLIST));
}

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(TClient *_client, EChannel *_chan, bool _owner, bool _op)
	: ECBPlayer(_chan, _owner, _op), client(_client)
{

}

const char* ECPlayer::GetNick() const
{
	return (client ? client->GetNick() : NULL);
}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

EChannel::EChannel(std::string _name)
	: ECBChannel(_name)
{
	limite = app.GetConf()->DefLimite(); /* Limite par default */
	app.NBchan++;
	app.NBwchan++;
	app.NBtotchan++;
}

EChannel::~EChannel()
{
	app.NBchan--;
	if(state == EChannel::WAITING) app.NBwchan--;
	else app.NBachan--;
	for (ChannelVector::iterator it = ChanList.begin(); it != ChanList.end(); )
	{
		if (*it == this)
		{
			it = ChanList.erase(it);
			return;
		}
		else
			++it;
	}
}

void EChannel::SetMap(ECBMap *m)
{
	/** \note Send infos like this :
	 * <pre>
	 * :server SET +l 5
	 * :p1,p2 SET -p
	 * SMAP line1
	 * SMAP line2
	 * [...]
	 * SMAP linex
	 * EOSMAP
	 * :p SET +m 0
	 * </pre>
	 */
	map = m;

	/* Envoie un peu brut mais pour redéfinir la limite */
	sendto_players(NULL, ":%s SET +l %d", app.GetConf()->ServerName().c_str(), m->MaxPlayers());
	SetLimite(m->MaxPlayers());

	/* Envoie d'une map */
	std::vector<std::string> map_file = m->MapFile();
	for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
		sendto_players(NULL, app.rpl(ECServer::SENDMAP), FormatStr((*it).c_str()));
	sendto_players(NULL, app.rpl(ECServer::ENDOFSMAP));
	return;
}

void EChannel::SetLimite(unsigned int l)
{
	limite = l;
	
	PlayerVector plv;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((*it)->Position() > limite)
		{
			(*it)->SetPosition(0);
			plv.push_back(dynamic_cast<ECPlayer*> (*it));
		}
	if(!plv.empty())
		send_modes(plv, "-p");
	return;
}

void EChannel::NeedReady()
{
	PlayerVector plv;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
	{
		if((*it)->Ready())
		{
			(*it)->SetReady(false);
			plv.push_back(dynamic_cast<ECPlayer*> (*it));
		}
	}
	send_modes(plv, "-!");

	return;
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((dynamic_cast<ECPlayer*> (*it))->Client() && !strcasecmp((*it)->GetNick(), nick))
			return ((ECPlayer*) (*it));
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((dynamic_cast<ECPlayer*> (*it))->Client() == cl)
			return ((ECPlayer*) (*it));

	return NULL;
}

void EChannel::send_modes(ECPlayer *sender, const char* msg)
{
	PlayerVector plv;
	plv.push_back(sender);

	send_modes(plv, msg);
	return;
}

void EChannel::send_modes(PlayerVector senders, const char* msg)
{
	if(senders.empty()) return;

	std::string snds;
	
	for(PlayerVector::iterator it = senders.begin(); it != senders.end(); ++it)
	{
		if(!snds.empty())
			snds += ",";
		snds += (*it)->GetNick();
	}
	sendto_players(NULL, app.rpl(ECServer::SET), snds.c_str(), msg);
	return;
}

int EChannel::sendto_players(ECPlayer* one, const char* pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
	{
		if(!(dynamic_cast<ECPlayer*> (*it))->Client() || *it == one) continue;

		(dynamic_cast<ECPlayer*> (*it))->Client()->sendbuf(buf, len);
	}
	return 0;
}
