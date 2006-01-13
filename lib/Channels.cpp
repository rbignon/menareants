/* lib/Channels.cpp - Game's channel of client AND server
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
#include "Outils.h"

/********************************************************************************************
 *                               ECBPlayer                                                   *
 ********************************************************************************************/

ECBPlayer::ECBPlayer(ECBChannel *_chan, bool _owner)
	: chan(_chan), owner(_owner)
{
	chan->AddPlayer(this);
	ready = false;
	fric = 0;
	place = 0;
	color = 0;
	map = 0;
}

bool ECBPlayer::SetPlace(unsigned int p)
{
	if(p <= chan->NbPlayers())
	{
		place = p;
		return true;
	}
	return false;
}

/********************************************************************************************
 *                               ECBChannel                                                   *
 ********************************************************************************************/

ECBChannel::ECBChannel(std::string _name)
	: name(_name)
{
	state = WAITING;
	limite = 0;
}

bool ECBChannel::AddPlayer(ECBPlayer* pl)
{
	if(!pl) return false;

	players.push_back(pl);
	return true;
}

bool ECBChannel::RemovePlayer(ECBPlayer* pl, bool use_delete)
{
	for (std::vector<ECBPlayer*>::iterator it = players.begin(); it != players.end(); )
	{
		if (*it == pl)
		{
			if(use_delete)
				delete pl;
			it = players.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

/* Lors de rajouts de modes, modifier API paragraphe 4. Modes */
const char* ECBChannel::ModesStr() const
{
	std::string modes = "+", params = "";
	if(limite) modes += "l", params += " " + TypToStr(limite);
	switch(state)
	{
		case WAITING: modes += "W"; break;
		case SENDING: modes += "S"; break;
		case PLAYING: modes += "P"; break;
		case ANIMING: modes += "A"; break;
	}
	/* Pas d'espace n�cessaire ici, rajout� � chaques fois qu'on ajoute un param */
	return (modes + params).c_str();
}

/* En cas de modification de la syntaxe, modifier � tout prix API paragraphe 5. PLS
 * Note qu'il faut �viter les incompatibilit�s � tous prix, et ne pas oublier, dans le cas
 * o� il y en a une, d'incr�menter le protocole
 */
const char* ECBChannel::PlayerList() const
{
	std::string list = "";
	for(unsigned int i=0; i<players.size();i++)
	{
		if(!list.empty()) list += " ";
		if(players[i]->IsOwner())
			list += "@";
		if(players[i]->Ready())
			list += "!";

		/* Informe de la place et de la couleur */
		list += TypToStr(players[i]->Place()) + "," + TypToStr(players[i]->Color()) + ",";

		list += players[i]->GetNick();
	}
	return list.c_str();
}
