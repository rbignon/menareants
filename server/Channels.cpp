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
				if(!sender->IsOwner())
					return Debug(W_DESYNCH, "SET %c%c d'un non owner", add ? '+' : '-', parv[1][i]);

				if(add)
				{
					if(j<parv.size())
					{
						uint mapi = StrToTyp<uint>(parv[j++]);
						if(mapi >= MapList.size())
							Debug(W_DESYNCH, "SET +m de la map %d hors de la liste (%d)", mapi, MapList.size());
						else
						{
							ECMap* map = MapList[mapi];
							if(map == sender->Channel()->Map())
								continue; /* anti-flood */
							sender->Channel()->SetMap(map);
							changed = YES_WITHPARAM;
						}
					}
				}
				else
					Debug(W_DESYNCH, "SET -m interdit.");
				break;
			case '!':
				/* Autorise seulement � se d�clarer comme OK, ne peut en aucun cas retirer ce qu'il
				 * a dit par la suite.
				 */
				if(sender->Ready())
				{
					Debug(W_WARNING, "SET %c%c: sender->Ready()=TRUE", add ? '+' : '-', parv[1][i]);
					break;
				}
				if(!add)
					{ Debug(W_DESYNCH, "SET %c%c interdit.", add ? '+' : '-', parv[1][i]); break; }
				sender->SetReady(add);
				changed = YES_NOPARAMS;
				break;
			case 'o':
			{
				if(!sender->IsOwner())
					{ Debug(W_DESYNCH, "SET %c%c d'un non owner", add ? '+' : '-', parv[1][i]); break; }
				if(j>=parv.size()) { Debug(W_DESYNCH, "+o sans nick"); break; }
				ECPlayer *pl = sender->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH, "%s non trouv�", parv[(j-1)].c_str()); break;}
				pl->SetOwner(add);
				changed = YES_WITHPARAM;
				break;
			}
			case 'c':
				if(!sender) { Debug(W_DESYNCH, "%cc sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "+c sans couleur"); break; }
					uint color = StrToTyp<uint>(parv[j++]);
					if(color > 0)
					{
						BPlayerVector::iterator it;
						for(it = sender->Channel()->Players().begin();
						    it != sender->Channel()->Players().end() && (*it)->Color() != color;
						    it++);
						if(it != sender->Channel()->Players().end())
							{ Debug(W_DESYNCH, "+c d'une couleur d�j� utilis�e"); break; }
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
				if(!sender) { Debug(W_DESYNCH, "%cp sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "+p sans couleur"); break; }
					uint place = StrToTyp<uint>(parv[j++]);
					if(place > 0)
					{
						if(place > sender->Channel()->GetLimite())
						{
							Debug(W_DESYNCH, "+p %d > %d(limite)", place,
							                 sender->Channel()->GetLimite());
							break;
						}
						BPlayerVector::iterator it;
						for(it = sender->Channel()->Players().begin();
						    it != sender->Channel()->Players().end() && (*it)->Position() != place;
						    it++);
						if(it != sender->Channel()->Players().end())
							{ Debug(W_DESYNCH, "+p d'une position d�j� utilis�e"); break; }
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
				Debug(W_DESYNCH, "Reception d'un mode non support� (%c%c)", add ? '+' : '-', parv[1][i]);
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

	return 0;
}

/** A client wants to join a channel.
 *
 * Syntax: JOI nom
 */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* Ne peut �tre que sur un seul salon � la fois */
	if(cl->Player() || parv[1].empty())
		return vDebug(W_WARNING, "Essaye de joindre plusieurs salons", VSName(cl->GetNick())
		                          VSName(cl->Player()->Channel()->GetName()) VName(parv[1]));

	const char* nom = parv[1].c_str();
	EChannel* chan = NULL;
	ECPlayer* pl;

	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); it++)
		if(!strcasecmp((*it)->GetName(), nom))
		{
			chan = *it;
			break;
		}

	if(!chan)
	{ /* Cr�ation du salon */
		chan = new EChannel(nom);
		pl = new ECPlayer(cl, chan, true);
		ChanList.push_back(chan);
	}
	else
	{ /* Rejoins un salon existant */
		if(!chan->Joinable())
		{
			vDebug(W_WARNING, "Le client essaye de joindre un salon en jeu", VSName(chan->GetName())
			                  VSName(cl->GetNick()) VIName(chan->State()));
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		}
		if(chan->GetLimite() && chan->NbPlayers() >= chan->GetLimite())
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		pl = new ECPlayer(cl, chan, false);
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

	chan->sendto_players(0, app.rpl(ECServer::JOIN), cl->GetNick(), nom, "");

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
	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); it++)
		if((*it)->Joinable())
			cl->sendrpl(app.rpl(ECServer::GLIST), (*it)->GetName(), (*it)->NbPlayers(),
			                                      (*it)->GetLimite());

	return cl->sendrpl(app.rpl(ECServer::EOGLIST));
}

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(TClient *_client, EChannel *_chan, bool _owner)
	: ECBPlayer(_chan, _owner), client(_client)
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
}

EChannel::~EChannel()
{
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

	/* Envoie un peu brut mais pour red�finir la limite */
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
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); it++)
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
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); it++)
	{
		(*it)->SetReady(false);
		plv.push_back(dynamic_cast<ECPlayer*> (*it));
	}
	send_modes(plv, "-!");

	return;
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); it++)
		if((dynamic_cast<ECPlayer*> (*it))->Client() && !strcasecmp((*it)->GetNick(), nick))
			return ((ECPlayer*) (*it));
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); it++)
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
	
	for(PlayerVector::iterator it = senders.begin(); it != senders.end(); it++)
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

	for(BPlayerVector::iterator it=players.begin(); it != players.end(); it++)
	{
		if(!(dynamic_cast<ECPlayer*> (*it))->Client() || *it == one) continue;

		(dynamic_cast<ECPlayer*> (*it))->Client()->sendbuf(buf, len);
	}
	return 0;
}
