/* src/Channels.h - Header of Channels.cpp
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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
#include <map>

class EChannel;

struct nations_str_t {
  const char* name;
  const char* infos;
};
extern const struct nations_str_t nations_str[];

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
	ECPlayer(std::string nick, EChannel* chan, bool owner, bool op, bool IsMe, bool IsIA);

	~ECPlayer();

	struct BreakPoint
	{
		BreakPoint(ECase* _c, std::string _message)
			: c(_c), message(_message), sprite(0)
		{}

		ECase* c;
		std::string message;
		ECSprite *sprite;
		ECImage text;
	};

/* Attributs */
public:

	/** Player's channel. */
	EChannel *Channel() const { return (EChannel*)ECBPlayer::Channel(); }

	/** Is this player me ? */
	bool IsMe() const { return isme; }

	virtual bool IsIA() const { return is_ia; }

	uint& Votes() { return votes; }

	void SetDisconnected(bool b = true)
	{
		ECBPlayer::SetDisconnected(b);
		votes = 0;
	}

	std::vector<BreakPoint> BreakPoints() const { return breakpoints; }
	void AddBreakPoint(BreakPoint bp);
	bool RemoveBreakPoint(ECBCase *c);

	typedef std::map<ECBEntity::e_type, std::map<int, ECSpriteBase*> > SpriteMap;
	SpriteMap Sprites() const { return sprites; }
	ECSpriteBase* GetSprite(ECBEntity::e_type e, int id) { return sprites[e][id]; }
	bool HasSprite(ECBEntity::e_type e, int id) { return sprites.find(e) != sprites.end() && sprites[e].find(id) != sprites[e].end(); }
	void SetSprite(ECBEntity::e_type e, int id, ECSpriteBase* sb) { sprites[e][id] = sb; }

/* Variables privées */
private:
	bool isme;
	bool is_ia;
	uint votes;
	SpriteMap sprites;
	std::vector<BreakPoint> breakpoints;
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

	EChannel(std::string _name, bool mission)
		: ECBChannel(_name, mission), want_leave(false), current_event(0)
	{}

/* Methodes */
public:

	void Print(std::string, int = 0x001); /* Par défault = I_Info */

/* Attributs */
public:

	/** Get a player in channel by his nickname. */
	ECPlayer* GetPlayer(const char* nick);

	ECPlayer* GetMe();

	ECMap* Map() const { return dynamic_cast<ECMap*>(ECBChannel::Map()); }

	bool WantLeave() const { return want_leave; }
	void SetWantLeave() { want_leave = true; }

	uint CurrentEvent() const { return current_event; }
	void SetCurrentEvent(uint e) { current_event = e; }

/* Variables privées */
private:
	bool want_leave;
	uint current_event;
};

#endif /* EC_CHANNELS_H */
