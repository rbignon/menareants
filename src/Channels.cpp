/* src/Channels.cpp - Channels functions
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
#include "Debug.h"

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(std::string _nick, EChannel *_chan, bool _owner, bool _op, bool _isme, bool _is_ia)
	: ECBPlayer(_nick, _chan, _owner, _op), isme(_isme), is_ia(_is_ia), votes(0)
{

}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

ECPlayer *EChannel::GetMe()
{
	BPlayerVector::iterator it;
	for(it=players.begin(); it != players.end() && !dynamic_cast<ECPlayer*>(*it)->IsMe(); ++it);

	return (it == players.end() ? 0 : (dynamic_cast<ECPlayer*>(*it)));
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	BPlayerVector::iterator it;
	for(it=players.begin(); it != players.end() && strcasecmp((*it)->GetNick(),nick); ++it);
	
	return (it == players.end() ? 0 : (dynamic_cast<ECPlayer*>(*it)));
}
