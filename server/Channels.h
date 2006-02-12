/* server/Channels.h - Header of Channels.cpp
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

#ifndef ECD_CHANNELS_H
#define ECD_CHANNELS_H

#include "../lib/Channels.h"

class TClient;

class ECBPlayer;

class ECBChannel;

class EChannel;

/********************************************************************************************
 *                               ECPlayers                                                  *
 ********************************************************************************************/
/** Derivation of class ECBPlayer for server.
 *
 * \note See definition of ECBPlayer to see all members of ECPlayers (in doc/lib/)
 */
class ECPlayer : public ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:

	/** Create a Player class in a channel.
	 * @param cl client struct of user who joined channel.
	 * @param chan channel struct.
	 * @param owner if player is an owner or not.
	 */
	ECPlayer(TClient* cl, EChannel* chan, bool owner);

/* Methodes */
public:

/* Attributs */
public:

	/** Return player's channel */
	EChannel *Channel() { return (EChannel*)chan; }

	/** Get client's structure of player */
	TClient *Client() { return client; }

	/** Get client's nickname */
	virtual const char* GetNick() const;

/* Variables privées */
protected:
	TClient *client;
};
typedef std::vector<ECPlayer> PlayerVector;

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

	EChannel(std::string _name);
	~EChannel();

/* Methodes */
public:

	/** All players have to be ready
	 * This will \b not send a message, it will only set all players as Ready = false.
	 */
	void NeedReady();

	/** Send a message to all players.
	 * 
	 * @param one if not null, we will not send message to \b this player.
	 * @return always 0.
	 */
	int sendto_players(ECPlayer* one, const char*, ...);

/* Attributs */
public:

	/** Get a player structure with his nickname. */
	ECPlayer* GetPlayer(const char* nick);

	/** Get a player structure with his client structure. */
	ECPlayer *GetPlayer(TClient *cl);
	
/* Variables privées */
protected:

};

typedef std::vector<EChannel*> ChannelVector;

extern ChannelVector ChanList;

#endif /* ECD_CHANNELS_H */
