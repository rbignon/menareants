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

class ECBChannel;
class ECBMap;
class ECBMapPlayer;
class ECBEntity;

typedef std::vector<ECBEntity*> BEntityVector;

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
	ECBPlayer(ECBChannel *_chan, bool _owner, bool _op);
	virtual ~ECBPlayer() {}

/* Méthodes */
public:

/* Attributs */
public:

	/** Player's Channel. */
	ECBChannel *Channel() { return chan; }

	uint Money() { return money; }                     /**< This is player's money. */
	void UpMoney(uint m) { money += m; }               /**< Add some money. */
	void DownMoney(uint m) { money -= m; }             /**< Remove some money */

	bool IsOwner() const { return owner; }             /**< Is he the owner of game ? */
	void SetOwner(bool o = true) { owner = o; }        /**< Set a player as the owner */

	bool IsOp() const { return op; }                   /**< Is he an operator of channel ? */
	void SetOp(bool o = true) { op = o; }              /**< Set a player as an oper */

	bool IsPriv() const { return op || owner; }        /**< Return true if player is op or owner */

	unsigned int Position() const { return position; } /**< Position of player in the map. */
	bool SetPosition(unsigned int p);                  /**< Set position of player. */

	/** Return color of player.
	 * \note Les couleurs sont représentées par un énumérateur. Alors
	 * il faut voir si le serveur en aurra quelque chose à foutre
	 * ou pas, si oui on met l'énumérateur ici, sinon on le met
	 * uniquement dans le client.
	 *
	 * \todo il faut mettre dans le client un tableau de couleurs SDL
	 */
	unsigned int Color() const { return color; }
	bool SetColor(unsigned int c);                      /**< Set color of player. */

	/** Return nick of player. */
	virtual const char* GetNick() const = 0;

	bool Ready() const { return ready; }                /**< Is player ready ? */
	void SetReady(bool r = true) { ready = r; }         /**< Set player. */

	ECBMapPlayer* MapPlayer() { return mp; }
	void SetMapPlayer(ECBMapPlayer* _mp) { mp = _mp; }

	ECList<ECBEntity*> *Entities() { return &entities; }

/* Variables privées */
protected:
	ECBChannel *chan;
	bool owner;
	bool op;
	unsigned int position;
	unsigned int color;
	uint money;
	ECBMapPlayer* mp;
	bool ready;
	ECList<ECBEntity*> entities;
};
typedef std::vector<ECBPlayer*> BPlayerVector;

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
		ANIMING            /**< Game is in animation. */
	};

/* Methodes */
public:

/* Attributs */
public:

	/** Return channel name. */
	const char* GetName() const { return name.c_str(); }

	/* A propos des etats de la partie */
	e_state State() const { return state; }                 /**< Return state of game. */
	bool IsInGame() const { return (state >= PLAYING); }    /**< Check if channel is in game. */
	bool Joinable() const { return (state == WAITING); }    /**< Check if channel is joinable. */
	void SetState(e_state s) { state = s; }                 /**< Define state. */

	/* Limite maximale pour entrer dans le chan */
	unsigned int GetLimite() const { return limite; }       /**< Return user limit of channel. */
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
	bool RemovePlayer(ECBPlayer* pl, bool use_delete);

	/** Return number of players in channel. */
	unsigned int NbPlayers() const { return players.size(); }

	/** Return aformated list of players.
	 * It is used to send to client a player list when it joined.
	 *
	 * \return formated list of players.
	 *
	 * \note Pour plus d'informations sur la syntaxe, consulter API paragraphe 5. PLS
	 */
	const char* PlayerList();

	/** Return a formated modes list.
	 * It is used to send to client a mode list when it joined.
	 *
	 * \return formated list of modes.
	 *
	 * \note Pour plus d'informations sur les modes, consulter API paragraphe 4. Modes
	 */
	const char* ModesStr() const;

/* Variables privées */
protected:
	BPlayerVector players;
	std::string name;
	e_state state;
	unsigned int limite;
	ECBMap *map;
};

#endif /* ECLIB_CHANNELS_H */
