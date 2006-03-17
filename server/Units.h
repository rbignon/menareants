/* server/Units.h - Header of Units.cpp
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

#ifndef ECD_UNITS_H
#define ECD_UNITS_H

#include "server/Map.h"
#include "lib/Units.h"

class ECArmy : public ECEntity, public ECBArmy
{
/* Constructeur/Destructeur */
public:
	ECArmy(const Entity_ID _name, ECBPlayer* _owner, ECase* _case, uint _nb)
		: ECBEntity(_name, _owner, _case, E_ARMY, ARMY_STEP, _nb)
	{}

/* Methodes */
public:

	virtual bool Attaq(std::vector<ECEntity*> entities);

/* Attributs */
public:

/* Variables protégées */
protected:
};

#endif /* ECD_UNITS_H */
