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
 * $Id$
 */

#ifndef ECLIB_CHANNELS_H
#define ECLIB_CHANNELS_H

#include <string>
#include <vector>

/********************************************************************************************
 *                               ECPlayer                                                   *
 ********************************************************************************************/

class TClient;

class ECPlayer
{
/* Constructeurs/Deconstructeurs */
public:
	ECPlayer() {}
	~ECPlayer() {}

/* Méthodes */
public:

/* Attributs */
public:

	/* Obtient l'objet du client du *serveur* !! */
	TClient *ServerClient() { return SClient; }

	/* Obtenir le pseudo du joueur */
	std::string GetNick() { return nick; }

/* Variables privées */
protected:
	TClient *SClient;

	std::string nick;
};

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

class EChannel
{
/* Constructeurs/Deconstructeurs */
public:

	/* Constructeur en donnant le nom du salon */
	EChannel(std::string _name);

	/* Deconstructeur par default */
	~EChannel() {}

/* Methodes */
public:

/* Attributs */
public:

	/* Obtient le nom du channel */
	std::string GetName() { return name; }

	/* Obtient un joueur par son pseudo */
	ECPlayer *GetPlayer(std::string);

	/* Ajoute un player */
	bool AddPlayer(ECPlayer*);

	/* Supprime un player
	 * use_delete: si true, supprime lui meme le ECPlayer
	 */
	bool RemovePlayer(ECPlayer*, bool use_delete);

/* Variables privées */
protected:
	std::string name;
	std::vector<ECPlayer*> players;
};

#endif /* ECLIB_CHANNELS_H */
