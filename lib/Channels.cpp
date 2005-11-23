/* lib/Channels.cpp - Game's channel of client AND server
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

/********************************************************************************************
 *                               ECBPlayer                                                   *
 ********************************************************************************************/

ECBPlayer::ECBPlayer(ECBChannel *_chan, bool _owner)
	: chan(_chan), owner(_owner)
{
	chan->AddPlayer(this);
}

/********************************************************************************************
 *                               ECBChannel                                                   *
 ********************************************************************************************/

ECBChannel::ECBChannel(std::string _name)
	: name(_name)
{

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

const char* ECBChannel::PlayerList()
{
	std::string list = "";
	for(unsigned int i=0; i<players.size();i++)
	{
		if(!list.empty()) list += " ";
		if(players[i]->IsOwner())
			list += "@";
		list += players[i]->GetNick();
	}
	return list.c_str();
}
