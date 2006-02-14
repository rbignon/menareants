/* lib/Map.h - Header of Map.cpp
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

#ifndef ECLIB_MAP_H
#define ECLIB_MAP_H

#include "Channel.h"
#include <vector>

/********************************************************************************************
 *                               ECBPlayer                                                  *
 ********************************************************************************************/

class ECBCase
{
/* Constructeur/Destructeur */
public:

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/********************************************************************************************
 *                               ECBMap                                                     *
 ********************************************************************************************/

class ECBMap
{
/* Constructeur/Destructeur */
public:

	/** Constructor
	 * @param _name name of map
	 * @param min min players to play
	 * @param max max players to play
	 * @param x x cases
	 * @param y y cases
	 */
	ECBMap(std::string _name, uint min, uint max, uint x, uint y);

/* Methodes */
public:

/* Attributs */
public:

	/** Name of channel */
	std::string Name() { return name; }

	/** Channel linked to map */
	ECBChannel* Channel() { return chan; }

/* Variables privées */
protected:
	ECBCase **map;
	ECBChannel *chan;
	std::string name;
	uint x, y;
	uint min, max;
};

#endif /* ECLIB_MAP_H */
