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
#include "Map.h"

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
	 * @param op if player is an operator or not.
	 */
	ECPlayer(TClient* cl, EChannel* chan, bool owner, bool op);

/* Methodes */
public:

	/** This player need to be ready. */
	void NeedReady();

/* Attributs */
public:

	virtual void UpMoney(int m) { SetMoney(money + m); }
	virtual void DownMoney(int m) { SetMoney(money - m); }
	void SetMoney(int m);

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

	EChannel(std::string _name);
	virtual ~EChannel();

/* Methodes */
public:

	/** All players have to be ready */
	void NeedReady();

	/** Send a message to all players.
	 * 
	 * @param one if not null, we will not send message to \b this player.
	 * @return always 0.
	 */
	int sendto_players(ECPlayer* one, const char*, ...);
	
	/** Send modes to all players from \a sender
	 * \warning this will only send modes, no set them in channel or on player.
	 */
	void send_modes(ECPlayer *sender, const char* msg);

	/** Send modes to all players from \a senders
	 * @param senders this will a player vector of senders.
	 * @param msg message to send
	 *
	 * \warning this will only send modes, no set them in channel of on players.
	 */
	void send_modes(PlayerVector senders, const char* msg);

	/** This function will return a name doesn't used before */
	const char* FindEntityName(ECPlayer*);

	/** This function send to players a ARM reply.
	 * @param cl put a TClient here if you want to send message only to  a list of clients.
	 * @param et this is a vector of entities who do an event.
	 * @param flags there is a lot of flags. You can show them at the end of comment.
	 * @param x an horizontal position. (optional)
	 * @param y a vertical position. (optional)
	 * @param nb number of units in entity. (optional)
	 * @param type type of entity. (optional)
	 *
	 * Note that params \a x \a y \a nb and \a type aren't necessary and you have to put
	 * a good flag to call them in function.
	 *
	 * <pre>
	 * Flags: (Definition in Map.h before ECEvent)
	 * ARM_MOVE      0x001
	 * ARM_SPLIT     0x002
	 * ARM_ATTAQ     0x004
	 * ARM_REMOVE    0x008
	 * ARM_LOCK      0x010
	 * ARM_TYPE      0x020
	 * ARM_NUMBER    0x040
	 * ARM_RETURN    0x080
	 * ARM_HIDE      0x100  (if you want to hide informations to players who haven't any entity in \a et list)
	 * ARM_RECURSE   0x200  (NEVER CALL IT)
	 * ARM_UNION     (ARM_MOVE|ARM_NUMBER)
	 * ARM_CREATE    (ARM_MOVE|ARM_TYPE|ARM_NUMBER)
	 * </pre>
	 */
	void SendArm(std::vector<TClient*> cl, std::vector<ECEntity*> et, uint flags,
	             uint x=0, uint y=0, uint nb=0, uint type=0);

	/** \see SendArm() */
	void SendArm(TClient* cl, std::vector<ECEntity*> et, uint flags, uint x=0, uint y=0, uint nb=0, uint type=0);
	void SendArm(TClient* cl, ECEntity* et, uint flags, uint x = 0, uint y = 0, uint nb = 0, uint type = 0);
	void SendArm(std::vector<TClient*> cl, ECEntity* et, uint flags, uint x=0, uint y=0, uint nb=0, uint type=0);

	void InitAnims();

	void NextAnim();
	
/* Attributs */
public:

	/** Get a player structure with his nickname. */
	ECPlayer* GetPlayer(const char* nick);

	/** Get a player structure with his client structure. */
	ECPlayer *GetPlayer(TClient *cl);

	/** Define the map */
	virtual void SetMap(ECBMap *m);
	
	/** Define user limit of channel.
	 * \warning This function will send to all players, if they have a position bigger than new limite, a
	 * SET message to set their position to 0 !
	 */
	virtual void SetLimite(unsigned int l);

	ECMap *Map() const { return (ECMap*)map; }
	
/* Variables privées */
protected:

};

typedef std::vector<EChannel*> ChannelVector;

extern ChannelVector ChanList;

#endif /* ECD_CHANNELS_H */
