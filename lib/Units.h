/* lib/Units.h - Header of Units.cpp
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

#ifndef ECLIB_UNITS_H
#define ECLIB_UNITS_H

#include "Map.h"

/********************************************************************************************
 *                               ECBArmy                                                   *
 ********************************************************************************************/
#define ARMY_STEP                  2
#define ARMY_NB                    100
#define ARMY_COST                  2000
#define ARMY_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_ARMY, ARMY_COST)
#define ARMY_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = ARMY_NB) :  ECBEntity(_name, _owner, _case, E_ARMY, ARMY_STEP, ARMY_COST, _nb)
/** This is a simple army */
class ECBArmy : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

  ARMY_EMPTY_CONSTRUCTOR(ECBArmy) {}

  ARMY_CONSTRUCTOR(ECBArmy) {}

	virtual ~ECBArmy() {}

/* Constantes */
public:

	bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
				return true;
			default:
				return false;
		}
	}

	uint InitNb() const { return ARMY_NB; }

/* Methodes */
public:

	virtual bool CanCreate(const ECBEntity*) { return false; }

/* Attributs */
public:

/* Variables privées */
protected:
};

#endif /* ECLIB_UNITS_H */
