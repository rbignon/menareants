/* lib/Channels.h - Header of Channels.cpp
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

class ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:
	ECBPlayer(ECBChannel *_chan, bool _owner);
	~ECBPlayer() {}

/* Méthodes */
public:

/* Attributs */
public:

	/* Salon auquel appartient le player */
	ECBChannel *Channel() { return chan; }

	/* Argent détenu par le joueur */
	int fric;

	/* Est le créateur de la partie  ? */
	bool IsOwner() { return owner; }

/* Variables privées */
protected:
	ECBChannel *chan;
	bool owner;
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

/* Methodes */
public:

/* Attributs */
public:

	/* Obtient le nom du channel */
	std::string GetName() { return name; }

	/* Récupère la liste des joueurs */
	std::vector<ECBPlayer*> Players() { return players; }

	/* Ajoute un player */
	bool AddPlayer(ECBPlayer*);

	/* Supprime un player
	 * use_delete: si true, supprime lui meme le ECPlayer
	 */
	bool RemovePlayer(ECBPlayer*, bool use_delete);

/* Variables privées */
protected:
	std::string name;
	std::vector<ECBPlayer*> players;
};

#endif /* ECLIB_CHANNELS_H */
