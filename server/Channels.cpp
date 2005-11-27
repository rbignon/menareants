/* server/Channels.cpp - Channels functions
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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
#include "Main.h"
#include <cstdarg>

std::vector<EChannel*> ChanList;

/********************************************************************************************
 *                              Commandes                                                   *
 ********************************************************************************************/

/* JOI <nom> */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* Ne peut être que sur un seul salon à la fois */
	if(cl->Player()) return cl->exit(app.rpl(ECServer::ERR));

	const char* nom = parv[1].c_str();
	EChannel* chan = NULL;
	ECPlayer* pl;
	unsigned int i;

	for(i=0; i<ChanList.size();i++)
		if(!strcasecmp(ChanList[i]->GetName(), nom))
		{
			chan = ChanList[i];
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
		pl = new ECPlayer(cl, chan, false);
	}
	cl->SetPlayer(pl);
	chan->sendto_players(0, app.rpl(ECServer::JOIN), cl->GetNick(), nom, "");
	cl->sendrpl(app.rpl(ECServer::SETS), "");
	cl->sendrpl(app.rpl(ECServer::PLIST), chan->PlayerList());
	return 0;
}

/* LEA [raison] */
int LEACommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* N'est pas dans un salon TODO: Desynch */
	if(!cl->Player()) return cl->exit(app.rpl(ECServer::ERR));

	const char* raison = parv.size() > 1 ? parv[1].c_str() : "";

	EChannel *chan = cl->Player()->Channel();
	chan->sendto_players(0, app.rpl(ECServer::LEAVE), cl->GetNick(), FormatStr(raison));

	chan->RemovePlayer(cl->Player(), true);
	if(!chan->NbPlayers())
		delete chan;

	cl->ClrPlayer();

	return 0;
}

/* LSP */
int LSPCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	for(unsigned i=0; i<ChanList.size();i++)
		cl->sendrpl(app.rpl(ECServer::GLIST), ChanList[i]->GetName(), ChanList[i]->NbPlayers(), 0);

	return cl->sendrpl(app.rpl(ECServer::EOGLIST));
}

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(TClient *_client, EChannel *_chan, bool _owner)
	: ECBPlayer(_chan, _owner), client(_client)
{

}

const char* ECPlayer::GetNick()
{
	return (client ? client->GetNick() : NULL);
}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

EChannel::~EChannel()
{
	for (std::vector<EChannel*>::iterator it = ChanList.begin(); it != ChanList.end(); )
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
	for(unsigned int i=0;i<players.size();i++)
		((ECPlayer *)players[i])->Ready = false;
	return;
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	for(unsigned int i=0;i<players.size();i++)
		if(((ECPlayer *)players[i])->Client() &&
		   !strcasecmp(((ECPlayer *)players[i])->GetNick(), nick))
			return ((ECPlayer *)players[i]);
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(unsigned int i=0;i<players.size();i++)
		if(((ECPlayer *)players[i])->Client() == cl)
			return ((ECPlayer *)players[i]);
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

	for(unsigned int i=0; i<players.size(); i++)
	{
		if(!((ECPlayer *)players[i])->Client() || ((ECPlayer *)players[i]) == one) continue;

		((ECPlayer *)players[i])->Client()->sendbuf(buf, len);
	}
	return 0;
}
