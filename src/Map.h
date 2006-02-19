/* src/Map.h - Header of Map.cpp
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

#ifndef EC_MAP_H
#define EC_MAP_H

#include "lib/Map.h"
#include "tools/Images.h"
#include <vector>

class ECMap : public ECBMap
{
/* Constructeur/Destructeur */
public:
	/** Path of map (used only by Server) */
	ECMap(std::string _filename);

	/** Constructor from a string's vector
	 * @param _map_file this is a string's vector where there is informations about map
	 */
	ECMap(std::vector<std::string> _map_file);

	virtual ~ECMap();

/* Attributs */
public:

	ECImage* Preview() { return preview; }            /**< Creation of map preview */

/* Variables privées */
protected:
	ECImage *preview;

	void CreatePreview();
};

typedef ECBCase        ECase;
typedef ECBVille       ECVille;
typedef ECBMer         ECMer;
typedef ECBTerre       ECTerre;
typedef ECBPont        ECPont;
typedef ECBMapPlayer   ECMapPlayer;
typedef ECBCountry     ECountry;
typedef ECBDate        ECDate;


#endif /* EC_MAP_H */
