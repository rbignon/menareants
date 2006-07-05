/* src/Channels.h - Header of Channels.cpp
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

#ifndef EC_CHANNELS_H
#define EC_CHANNELS_H

#include "../lib/Channels.h"
#include "Map.h"

class EChannel;

/********************************************************************************************
 *                               ECPlayers                                                  *
 ********************************************************************************************/
/** Derivation of class ECBPlayer for client game.
 *
 * \note See definition of ECBPlayer to see all members of ECPlayers (in doc/lib/)
 */
class ECPlayer : public ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:

	/** Create a Player class in a channel.
	 * @param nick nickname of user who joined channel.
	 * @param chan channel struct.
	 * @param owner if player is an owner or not.
	 * @param op is player is an oper or not.
	 * @param IsMe say if this player is me.
	 * @param IsIA say if this player is an Artificial Intelligence.
	 */
	ECPlayer(const char* nick, EChannel* chan, bool owner, bool op, bool IsMe, bool IsIA);

/* Methodes */
public:

/* Attributs */
public:

	/** Player's channel. */
	EChannel *Channel() const { return (EChannel*)chan; }

	/** Get nick of client. */
	virtual const char* GetNick() const { return nick.c_str(); }

	/** Is this player me ? */
	bool IsMe() const { return isme; }

	virtual bool IsIA() const { return is_ia; }

/* Variables privées */
protected:
	std::string nick;
	bool isme;
	bool is_ia;
};
typedef std::vector<ECPlayer*> PlayerVector;

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/
/** Channel structure based on ECBChannel
 *
 * \note See definition of ECBChannel to see all members of EChannel (in doc/lib/)
 */
class EChannel : public ECBChannel
{
/* Constructeurs/Deconstructeurs */
public:

	EChannel(std::string _name)
		: ECBChannel(_name), want_leave(false), current_event(0)
	{}

/* Methodes */
public:

	void Print(std::string, int = 0x001); /* Par défault = I_Info */

/* Attributs */
public:

	/** Get a player in channel by his nickname. */
	ECPlayer* GetPlayer(const char* nick);

	ECPlayer* GetMe();

	ECMap* Map() const { return dynamic_cast<ECMap*>(map); }

	bool WantLeave() const { return want_leave; }
	void SetWantLeave() { want_leave = true; }

	uint CurrentEvent() const { return current_event; }
	void SetCurrentEvent(uint e) { current_event = e; }

/* Variables privées */
protected:
	bool want_leave;
	uint current_event;
};

#endif /* EC_CHANNELS_H */
