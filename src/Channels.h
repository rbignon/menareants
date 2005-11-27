/* src/Channels.h - Header of Channels.cpp
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

#ifndef EC_CHANNELS_H
#define EC_CHANNELS_H

#include "../lib/Channels.h"

class EChannel;

/********************************************************************************************
 *                               ECPlayers                                                  *
 ********************************************************************************************/
class ECPlayer : public ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:

	/* Création en donnant la structure du client, du salon et
	 * en spécifiant si il est owner ou non, et si c'est moi ou non.
	 */
	ECPlayer(const char* nick, EChannel* chan, bool owner, bool IsMe);

/* Methodes */
public:

/* Attributs */
public:

	/* Salon auquel appartient le player */
	EChannel *Channel() { return (EChannel*)chan; }

	/* Obtient le pseudo du client */
	virtual const char* GetNick() { return nick.c_str(); }

	/* Est-ce que ce player est moi */
	bool IsMe() { return isme; }

/* Variables privées */
protected:
	std::string nick;
	bool isme;
};

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

class EChannel : public ECBChannel
{
/* Constructeurs/Deconstructeurs */
public:

	EChannel(std::string _name)
		: ECBChannel(_name)
	{}

	~EChannel() {}

/* Methodes */
public:

/* Attributs */
public:

	/* Récupère le Player par le pseudo */
	ECPlayer* GetPlayer(const char* nick);

/* Variables privées */
protected:

};

#endif /* EC_CHANNELS_H */
