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

#include "../lib/Channels.h"
#include "Channels.h"
#include "Server.h"

std::vector<EChannel*> ChanList;

/********************************************************************************************
 *                              Commandes                                                   *
 ********************************************************************************************/

/* JOI <nom> */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	std::string nom = parv[1];
	EChannel* chan;
	ECPlayer* pl;
	unsigned int i;

	for(i=0; i<ChanList.size() && (ChanList[i]->GetName() != nom || !(chan = ChanList[i];i++);

	if(!chan)
	{ /* Création du salon */
		chan = new EChannel(nom);
		pl = new ECPlayer(cl, chan);
		chan->AddPlayer(pl);
		cl->sendrpl(app.rpl(ECServer::JOIN), cl->nick, nom, "");
		cl->sendrpl(app.rpl(ECServer::SETS), "");
		cl->sendrpl(app.rpl(ECServer::PLIST), );
	}
	else
	{ /* Rejoins un salon existant */

	}
	return 0;
}

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(TClient *_client, EChannel *_chan)
	: ECBPlayer(_chan), client(_client)
{

}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

void EChannel::NeedReady()
{
	for(unsigned int i=0;i<players.size();i++)
		players[i]->Ready = false;
	return;
}

ECPlayer *EChannel::GetPlayer(char* nick)
{
	for(unsigned int i=0;i<players.size();i++)
		if(players[i]->Client() &&
		   !strcasecmp(players[i]->Client()->GetNick(), nick))
			return players[i];
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(unsigned int i=0;i<players.size();i++)
		if(players[i]->Client() == cl)
			return players[i];
	return NULL;
}
