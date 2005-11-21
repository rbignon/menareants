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
 * $Id$
 */

#include "Channels.h"

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

EChannel::EChannel(std::string _name)
	: name(_name)
{

}

ECPlayer *EChannel::GetPlayer(std::string nick)
{
	for(unsigned int i=0;i<players.size();i++)
		if(players[i]->GetNick() == nick)
			return players[i];
	return NULL;
}

bool EChannel::AddPlayer(ECPlayer* pl)
{
	if(!pl) return false;

	players.push_back(pl);
	return true;
}

bool EChannel::RemovePlayer(ECPlayer* pl, bool use_delete)
{
	for (std::vector<ECPlayer*>::iterator it = players.begin(); it != players.end(); )
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
