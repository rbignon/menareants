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

/********************************************************************************************
 *                               ECBPlayer                                                   *
 ********************************************************************************************/

class ECBChannel;

class ECBMap;

/* ATTENTION! Classe virtuelle */
class ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:
	ECBPlayer(ECBChannel *_chan, bool _owner);
	virtual ~ECBPlayer() {}

/* Méthodes */
public:

/* Attributs */
public:

	/* Salon auquel appartient le player */
	ECBChannel *Channel() { return chan; }

	/* Argent détenu par le joueur */
	int fric;

	/* Est le créateur de la partie  ? */
	bool IsOwner() const { return owner; }
	void SetOwner(bool o = true) { owner = o; }

	/* Place */
	unsigned int Place() const { return place; }
	bool SetPlace(unsigned int p);

	/* MAP */
	ECBMap *Map() const { return map; }
	void SetMap(ECBMap *m) { map = m; }

	/* Couleur
	 * Les couleurs sont représentées par un énumérateur... Alors
	 * il faut voir si le serveur en aurra quelque chose à foutre
	 * ou pas, si oui on met l'énumérateur ici, sinon on le met
	 * uniquement dans le client.
	 * TODO: il faut mettre dans le client un tableau de couleurs SDL
	 */
	unsigned int Color() const { return color; }
	void SetColor(unsigned int c) { color = c; }

	/* Retourne le pseudo du joueur */
	virtual const char* GetNick() const = 0;

	/* Le joueur est pret (utilisation de RDY) */
	bool Ready() const { return ready; }
	void SetReady(bool r = true) { ready = r; }

/* Variables privées */
protected:
	ECBChannel *chan;
	bool owner;
	unsigned int place;
	unsigned int color;
	ECBMap *map;
	bool ready;
};

/********************************************************************************************
 *                               ECBChannel                                                   *
 ********************************************************************************************/

class ECBChannel
{
/* Constructeurs/Deconstructeurs */
public:

	/* Constructeur en donnant le nom du salon */
	ECBChannel(std::string _name);

	/* Deconstructeur par default */
	~ECBChannel() {}

	enum e_state {
		WAITING,
		SENDING,
		PLAYING,
		ANIMING
	};

/* Methodes */
public:

/* Attributs */
public:

	/* Obtient le nom du channel */
	const char* GetName() const { return name.c_str(); }

	/* Récupère la liste des joueurs */
	std::vector<ECBPlayer*> Players() const { return players; }

	/* Ajoute un player */
	bool AddPlayer(ECBPlayer*);

	/* Supprime un player
	 * use_delete: si true, supprime lui meme le ECPlayer
	 */
	bool RemovePlayer(ECBPlayer*, bool use_delete);

	/* Retourne le nombre de joueurs dans le jeu */
	unsigned int NbPlayers() const { return players.size(); }

	/* Retourne la liste des joueurs
	 * Note: Pour plus d'informations sur la syntaxe, consulter API paragraphe 5. PLS
	 */
	const char* PlayerList() const;

	/* Retourne les modes formatés
	 * Note: Pour plus d'informations sur les modes, consulter API paragraphe 4. Modes
	 */
	const char* ModesStr() const;

	/* A propos des etats de la partie */
	e_state State() const { return state; }
	bool IsInGame() const { return (state >= PLAYING); }
	bool Joinable() const { return (state == WAITING); }
	void SetState(e_state s) { state = s; }

	/* Limite maximale pour entrer dans le chan */
	unsigned int GetLimite() const { return limite; }
	void SetLimite(unsigned int l) { limite = l; }

/* Variables privées */
protected:
	std::string name;
	std::vector<ECBPlayer*> players;
	e_state state;
	unsigned int limite;
};

#endif /* ECLIB_CHANNELS_H */

