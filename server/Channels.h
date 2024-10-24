/* server/Channels.h - Header of Channels.cpp
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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

#include "lib/Channels.h"
#include "lib/Messages.h"
#include "Map.h"

class TClient;
class ECBPlayer;
class ECBChannel;
class EChannel;
class ECEvent;

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

	struct BreakPoint
	{
		BreakPoint(ECBCase* _c, std::string _message)
			: c(_c), message(_message)
		{}

		ECBCase* c;
		std::string message;
	};

/* Methodes */
public:

	/** This player need to be ready. */
	void NeedReady();

	void HaveLost();

/* Attributs */
public:

	virtual void UpMoney(int m) { SetMoney(Money() + m); }
	virtual void DownMoney(int m) { SetMoney(Money() - m); }
	void SetMoney(int m);

	/** Return player's channel */
	EChannel *Channel() { return (EChannel*)ECBPlayer::Channel(); }

	/** Get client's structure of player */
	TClient *Client() { return client; }
	void ClearClient() { client = 0; }
	void SetClient(TClient* c) { client = c; }

	virtual bool IsIA() const;

	struct stats_t
	{
		stats_t() : killed(0), losses(0), created(0), score(0), best_revenu(0) {}
		uint killed;
		uint losses;
		uint created;
		int score;
		int best_revenu;
	};

	stats_t Stats() const { return stats; }
	stats_t* Stats() { return &stats; }

	std::vector<TClient*> ClientAllies() const;

	BPlayerVector::size_type Votes() const { return votes.size(); }
	bool AddVote(ECBPlayer* pl);

	void AddAllie(ECBPlayer*);

	void SetDisconnected(bool b = true)
	{
		ECBPlayer::SetDisconnected(b);
		votes.clear();
	}

	std::vector<BreakPoint> BreakPoints() const { return breakpoints; }
	void AddBreakPoint(BreakPoint bp) { breakpoints.push_back(bp); }
	bool RemoveBreakPoint(ECBCase *c);

	ECList<ECEvent*>* Events() { return &events; }

	void CalculBestRevenu(int revenu) { if(revenu > stats.best_revenu) stats.best_revenu = revenu; }

	void SetCookie(std::string c) { cookie = c; }
	std::string Cookie() const { return cookie; }

/* Variables privées */
private:
	TClient *client;
	stats_t stats;
	BPlayerVector votes;
	std::vector<BreakPoint> breakpoints;
	ECList<ECEvent*> events;
	int best_revenu;
	std::string cookie;
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

	EChannel(std::string _name, bool mission);
	virtual ~EChannel();

	enum info_messages
	{
		I_NONE,
		I_SHOOT,  // attaquant, attaqué, domage
		I_JOUANO,  // jouano, nom_exowner_du_mcdo, caserne_investie, nb_de_tours
		I_DEBUG,
		I_JOUANO_FART, // jouano
		I_GULAG_GAIN, // gulag
	};

/* Methodes */
public:

	/** All players have to be ready */
	void NeedReady();

	/** Send a message to all players.
	 *
	 * @param one if not null, we will not send message to \b this player.
	 * @param from this is the sender of this message (it can be null).
	 * @param command command
	 * @param args arguments
	 * @return always 0.
	 */
	int sendto_players(ECPlayer* one, ECBPlayer* from, ECMessage command, ECArgs args = ECArgs());
	int sendto_players(ECPlayer* one, PlayerVector from, ECMessage, ECArgs = ECArgs());
	int sendto_players(ECPlayer* one, std::vector<ECEntity*> from, ECMessage, ECArgs = ECArgs());
	int sendto_players(ECPlayer* one, std::string from, ECMessage, ECArgs = ECArgs());

	/** Send an INFO message to someone
	 * @param pl player will receive message. if null, all players will receive message.
	 * @param id \b info_messages enumerator to identificate message
	 * @param args arguments used to format message
	 */
	void send_info(ECPlayer* pl, info_messages id, ECArgs args);

	/** Send modes to all players from \a sender
	 * \warning this will only send modes, no set them in channel or on player.
	 */
	void send_modes(ECPlayer *sender, std::string modes, ECArgs = ECArgs());

	/** Send modes to all players from \a senders
	 * @param senders this will a player vector of senders.
	 * @param msg message to send
	 *
	 * \warning this will only send modes, no set them in channel or on players.
	 */
	void send_modes(PlayerVector senders, std::string modes, ECArgs = ECArgs());

	/** This function will return a name doesn't used before */
	const char* FindEntityName(ECPlayer*);

	/** This function send to players a ARM reply.
	 * @param cl put a TClient here if you want to send message only to  a list of clients.
	 * @param et this is a vector of entities who do an event.
	 * @param flags there is a lot of flags. You can show them at the end of comment.
	 * @param x an horizontal position. (optional)
	 * @param y a vertical position. (optional)
	 * @param data a data structure for ~ flag.
	 * @param events this is a vector of events (in exemple for an attaq)
	 *
	 * Note that params \a x \a y \a nb and \a type aren't necessary and you have to put
	 * a good flag to call them in function.
	 *
	 * <pre>
	 * Flags: (Definition in lib/Map.h before ECBEntity)
	 * ARM_MOVE        0x0001  (=id,x,r,[<>^v])
	 * ARM_UNUSED      0x0002  (UNUSED)
	 * ARM_ATTAQ       0x0004  (*x,y)
	 * ARM_REMOVE      0x0008  (-)
	 * ARM_LOCK        0x0010  (.)
	 * ARM_TYPE        0x0020  (%n)
	 * ARM_NUMBER      0x0040  (+n)
	 * ARM_RETURN      0x0080  (\<x,y)
	 * ARM_HIDE        0x0100  (if you want to hide informations to players who haven't any entity in \a et list)
	 * ARM_RECURSE     0x0200  (NEVER CALL IT)
	 * ARM_NOCONCERNED 0x0400  (if you don't want to send it to the others players of owner)
	 * ARM_DEPLOY      0x0800  ({ or })
	 * ARM_FORCEATTAQ  0x1000  (!, to force an attaq to a specific case)
	 * ARM_CONTENER    0x2000  ( ) )
	 * ARM_UNCONTENER  0x4000  ( ( )
	 * ARM_NOPRINCIPAL 0x8000  (&)
	 * ARM_DATA        0x10000 (~id,data)
	 * ARM_UPGRADE     0x20000 (U)
	 * ARM_PREUNION    (ARM_MOVE|ARM_LOCK)
	 * ARM_UNION       (ARM_MOVE|ARM_NUMBER)
	 * ARM_CREATE      (ARM_MOVE|ARM_TYPE|ARM_NUMBER)
	 * ARM_CONTAIN     (ARM_CONTENER|ARM_MOVE)
	 * ARM_UNCONTAIN   (ARM_UNCONTENER|ARM_MOVE)

	 * </pre>
	 */
	void SendArm(std::vector<TClient*> cl, std::vector<ECEntity*> et, uint flags,
	             uint x=0, uint y=0, ECData data = 0, std::vector<ECEvent*> events = std::vector<ECEvent*>(0));

	/** \see SendArm() */
	void SendArm(TClient* cl, std::vector<ECEntity*> et, uint flags, uint x=0, uint y=0, ECData data = 0,
	             std::vector<ECEvent*> events = std::vector<ECEvent*>(0));
	void SendArm(TClient* cl, ECEntity* et, uint flags, uint x = 0, uint y = 0, ECData data = 0,
	             std::vector<ECEvent*> events = std::vector<ECEvent*>(0));
	void SendArm(std::vector<TClient*> cl, ECEntity* et, uint flags, uint x=0, uint y=0, ECData data = 0,
	             std::vector<ECEvent*> events = std::vector<ECEvent*>(0));

	/** Initialization of animations (sort, etc) */
	void InitAnims();

	/** Show next animation */
	void NextAnim();

	/** Show an animation */
	bool ShowAnim(ECEvent* event);

	/** Return a formated modes list.
	 * It is used to send to client a mode list when it joined.
	 *
	 * \return formated list of modes.
	 *
	 * \note Pour plus d'informations sur les modes, consulter API paragraphe 4. Modes
	 */
	std::string ModesStr() const;

	/** Send a LEA message to all players and close this channel */
	void ByeEveryBody(ECBPlayer* exception = 0);

	/** Check if every body is ready, and do a lot of things if it is true */
	void CheckReadys();

	/** Check if this is the end of the game */
	bool CheckEndOfGame();

	bool CheckPinging();

	/** Send all entities to someone */
	void SendEntities(ECPlayer* pl);

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

	ECMap *Map() const { return static_cast<ECMap*>(ECBChannel::Map()); }

	bool RemovePlayer(ECBPlayer* pl, bool use_delete);

	/** This owner of this channel */
	ECPlayer* Owner() { return owner; }
	void SetOwner(ECPlayer* _o) { owner = _o; }

	/** Number of humans players in this channel */
	BPlayerVector::size_type NbHumains(bool all = false) const;

	bool FastGame() const { return fast_game; }
	void SetFastGame(bool b = true) { fast_game = b; }

	int& BeginMoney() { return begin_money; }

/* Variables privées */
protected:
	ECPlayer* owner;
	bool fast_game;
	int begin_money;
	uint first_playing, playing;
};

typedef std::vector<EChannel*> ChannelVector;

extern ChannelVector ChanList;

#endif /* ECD_CHANNELS_H */
