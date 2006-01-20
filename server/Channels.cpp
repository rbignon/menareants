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
#include <cstdarg>

std::vector<EChannel*> ChanList;

/********************************************************************************************
 *                              Commandes                                                   *
 ********************************************************************************************/

/* MSG <message> */
int MSGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player())
		return vDebug(W_DESYNCH, "MSG en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	cl->Player()->Channel()->sendto_players(cl->Player(),
			app.rpl(ECServer::MSG), cl->GetNick(), FormatStr(parv[1].c_str()));
	return 0;
}

/* SET <modes> [params ...] */
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
		{ 'l', false, false, NULL},
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
			case 'l':
				if(!sender->IsOwner())
					return Debug(W_DESYNCH, "SET %c%c d'un non owner", add ? '+' : '-', parv[1][i]);
				if(add)
				{
					if(j<parv.size())
					{
						sender->Channel()->SetLimite(StrToTyp<uint>(parv[j++]));
						changed = YES_WITHPARAM;
					}
					else Debug(W_DESYNCH, "+l sans limite");
				}
				else
				{
					sender->Channel()->SetLimite(0);
					changed = YES_NOPARAMS;
				}
				break;
			case '!':
				/* Autorise seulement à se déclarer comme OK, ne peut en aucun cas retirer ce qu'il
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
				if(!pl) { Debug(W_DESYNCH, "%s non trouvé", parv[(j-1)].c_str()); break;}
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
						PlayerIterator it;
						for(it = sender->Channel()->Players().begin();
						    it != sender->Channel()->Players().end() && (*it)->Color() != color;
						    it++);
						if(it != sender->Channel()->Players().end())
							{ Debug(W_DESYNCH, "+c d'une couleur déjà utilisée"); break; }
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
						PlayerIterator it;
						for(it = sender->Channel()->Players().begin();
						    it != sender->Channel()->Players().end() && (*it)->Place() != place;
						    it++);
						if(it != sender->Channel()->Players().end())
							{ Debug(W_DESYNCH, "+p d'une position déjà utilisée"); break; }
					}
					sender->SetPlace(place);
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetPlace(0);
					changed = YES_NOPARAMS;
				}
				break;
			default:
				Debug(W_DESYNCH, "Reception d'un mode non supporté (%c%c)", add ? '+' : '-', parv[1][i]);
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

/* JOI <nom> */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* Ne peut être que sur un seul salon à la fois */
	if(cl->Player() || parv[1].empty())
		return vDebug(W_WARNING, "Essaye de joindre plusieurs salons", VSName(cl->GetNick())
		                          VSName(cl->Player()->Channel()->GetName()));

	const char* nom = parv[1].c_str();
	EChannel* chan = NULL;
	ECPlayer* pl;

	for(ChannelIterator it=ChanList.begin(); it != ChanList.end(); it++)
		if(!strcasecmp((*it)->GetName(), nom))
		{
			chan = *it;
			break;
		}

	if(!chan)
	{ /* Création du salon */
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
			return cl->exit(app.rpl(ECServer::ERR));
		}
		if(chan->GetLimite() && chan->NbPlayers() >= chan->GetLimite())
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		pl = new ECPlayer(cl, chan, false);
	}
	cl->SetPlayer(pl);
	chan->sendto_players(0, app.rpl(ECServer::JOIN), cl->GetNick(), nom, "");
	cl->sendrpl(app.rpl(ECServer::SET), app.GetConf()->ServerName().c_str(), chan->ModesStr());
	cl->sendrpl(app.rpl(ECServer::PLIST), chan->PlayerList());
	return 0;
}

/* LEA */
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

/* LSP */
int LSPCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	for(ChannelIterator it=ChanList.begin(); it != ChanList.end(); it++)
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
	for (ChannelIterator it = ChanList.begin(); it != ChanList.end(); )
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

void EChannel::NeedReady()
{
	for(PlayerIterator it=players.begin(); it != players.end(); it++)
		(*it)->SetReady(false);
	return;
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	for(PlayerIterator it=players.begin(); it != players.end(); it++)
		if(((ECPlayer*) (*it))->Client() && !strcasecmp((*it)->GetNick(), nick))
			return ((ECPlayer*) (*it));
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(PlayerIterator it=players.begin(); it != players.end(); it++)
		if(((ECPlayer*) (*it))->Client() == cl)
			return ((ECPlayer*) (*it));
	return NULL;
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

	for(PlayerIterator it=players.begin(); it != players.end(); it++)
	{
		if(!((ECPlayer*) (*it))->Client() || *it == one) continue;

		((ECPlayer*) (*it))->Client()->sendbuf(buf, len);
	}
	return 0;
}
