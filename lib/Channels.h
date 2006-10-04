/* lib/Channels.h - Header of Channels.cpp
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

#ifndef ECLIB_CHANNELS_H
#define ECLIB_CHANNELS_H

#include <string>
#include <vector>
#include "Outils.h"
#include "Colors.h"

class ECBChannel;
class ECBMap;
class ECBMapPlayer;
class ECBEntity;
class ECBCase;

typedef std::vector<ECBEntity*> BEntityVector;

struct nations_str_t {
  const char* name;
  const char* infos;
}; 
extern const struct nations_str_t nations_str[];

class ECBPlayer;
typedef std::vector<ECBPlayer*> BPlayerVector;



/********************************************************************************************
 *                               ECBPlayer                                                  *
 ********************************************************************************************/
/** Base class of Player/
 * There are informations about a player whose are used by server and client.
 *
 * \attention This is a \a virtual class !
 */
class ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:
	ECBPlayer(std::string nick, ECBChannel *_chan, bool _owner, bool _op);
	virtual ~ECBPlayer();

	/** List of nations */
	enum e_nations
	{
		N_NONE,
		N_NOISY,
		N_USA,
		N_FRANCE,
		N_URSS,
		N_ALQUAIDA,
		N_ESPAGNE,
		N_JAPON,
		N_COLOMBIE,
		N_MAX
	};

/* Attributs */
public:

	/** Player's Channel. */
	ECBChannel *Channel() const { return chan; }

	int Money() const { return money; }                /**< This is player's money. */
	virtual void UpMoney(int m) { money += m; }        /**< Add some money. */
	virtual void DownMoney(int m) { money -= m; }      /**< Remove some money. */
	virtual void SetMoney(int m) { money = m; }        /**< Set money. */

	unsigned int Position() const { return position; } /**< Position of player in the map. */
	bool SetPosition(unsigned int p);                  /**< Set position of player. */

	/** Return color of player.
	 * \note Colors are an enumerator defined in Colors.h.
	 */
	unsigned int Color() const { return color; }
	bool SetColor(unsigned int c);                     /**< Set color of player. */

	unsigned int Nation() const { return nation; }
	bool SetNation(unsigned int n);

	/** Return nick of player. */
	virtual const char* GetNick() const { return nick.c_str(); }
	std::string Nick() const { return nick; }

	ECBMapPlayer* MapPlayer() const { return mp; }
	void SetMapPlayer(ECBMapPlayer* _mp) { mp = _mp; }

	ECList<ECBEntity*> *Entities() { return &entities; }

	BPlayerVector::size_type NbAllies() const { return allies.size(); }
	void AddAllie(ECBPlayer*);
	bool RemoveAllie(ECBPlayer*);
	bool IsAllie(ECBPlayer*) const;
	BPlayerVector Allies() const { return allies; }

	virtual bool IsIA() const = 0;

	bool Ready() const;                                 /**< Is player ready ? */
	void SetReady(bool r = true);                       /**< Set player. */

	bool IsOwner() const;                              /**< Is he the owner of game ? */
	void SetOwner(bool o = true);                      /**< Set a player as the owner */

	bool IsOp() const;                                 /**< Is he an operator of channel ? */
	void SetOp(bool o = true);                         /**< Set a player as an oper */

	bool IsPriv() const { return IsOp() || IsOwner(); }        /**< Return true if player is op or owner */

	bool Lost() const;
	void SetLost(bool b = true);

	bool Disconnected() const;
	virtual void SetDisconnected(bool b = true);

	bool CanRejoin() const { return (Disconnected() && !Lost()); }

protected:
	BPlayerVector allies;

/* Variables privées */
private:
	std::string nick;
	ECBChannel *chan;
	unsigned int position;
	unsigned int color;
	int money;
	ECBMapPlayer* mp;
	ECList<ECBEntity*> entities;
	unsigned int nation;
	void SetFlag(int f, bool b);
	int flags;
};

/********************************************************************************************
 *                               ECBChannel                                                 *
 ********************************************************************************************/

/** Base class of a Channel.
 * There are channel's informations used by server and client.
 */
class ECBChannel
{
/* Constructeurs/Deconstructeurs */
public:

	/** @param _name name of channel */
	ECBChannel(std::string _name);

	virtual ~ECBChannel();

	/** Define state of game */
	enum e_state {
		WAITING,           /**< Game has been created and is able to be joined. */
		SENDING,           /**< Informations about game are sending. */
		PLAYING,           /**< Players are playing. */
		ANIMING,           /**< Game is in animation. */
		PINGING,           /**< Game is paused because any players are disconnected because of a lost connexion */
		SCORING            /**< End of game. */
	};

	ECBChannel &operator=(const ECBChannel &src) { throw std::string("La copie de cette classe est interdite"); }

/* Methodes */
public:

/* Attributs */
public:

	/** Return channel name. */
	const char* GetName() const { return name.c_str(); }
	std::string Name() const { return name; }

	/* A propos des etats de la partie */
	e_state State() const { return state; }                 /**< Return state of game. */
	bool IsInGame() const { return (state >= PLAYING) && (state < SCORING); }    /**< Check if channel is in game. */
	bool Joinable() const { return (state == WAITING); }    /**< Check if channel is joinable. */
	bool IsPinging() const { return (state == PINGING); }   /**< Is this channel pinging ? */
	void SetState(e_state s) { state = s; }                 /**< Define state. */

	/* Limite maximale pour entrer dans le chan */
	unsigned int Limite() const { return limite; }          /**< Return user limit of channel. */
	virtual void SetLimite(unsigned int l) { limite = l; }  /**< Define user limit of channel. */

	/** Return MAP */
	ECBMap *Map() const { return map; }

	/** Define the map */
	virtual void SetMap(ECBMap *m) { map = m; }

	/** Return player list in the channel. */
	BPlayerVector Players() const { return players; }

	/** Add a player. */
	bool AddPlayer(ECBPlayer*);

	/** Delete a player.
	 * @param pl class of player to remove.
	 * @param use_delete if true, use \a delete on \a pl .
	 */
	virtual bool RemovePlayer(ECBPlayer* pl, bool use_delete);

	/** Return number of players in channel. */
	unsigned int NbPlayers() const { return players.size(); }

	/** Return aformated list of players.
	 * It is used to send to client a player list when it joined.
	 *
	 * \return formated list of players.
	 *
	 * \note Pour plus d'informations sur la syntaxe, consulter API paragraphe 5. PLS
	 */
	std::string PlayerList();

	uint TurnTime() const { return turn_time; }
	void SetTurnTime(uint t) { turn_time = t; }

	bool IsMission() const { return mission; }
	void SetMission(bool m = true) { mission = m; }

/* Variables privées */
protected:
	BPlayerVector players;

private:
	std::string name;
	e_state state;
	unsigned int limite;
	ECBMap *map;
	uint turn_time;
	bool mission;
};

#endif /* ECLIB_CHANNELS_H */
