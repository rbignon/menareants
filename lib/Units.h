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
/** This is a simple army */
#define ARMY_STEP         2
class ECBArmy : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ECBArmy(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb)
		: ECBEntity(_name, _owner, _case, E_ARMY, ARMY_STEP, _nb)
	{}

	ECBArmy() {}

	virtual ~ECBArmy() {}

/* Constantes */
public:

	bool CanAttaq(ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
				return true;
			default:
				return false;
		}
	}

/* Methodes */
public:

	/** @return last case */
	virtual ECBCase* Move(uint x, uint y);

	virtual bool Return();

	virtual ECBCase* Attaq(uint x, uint y);

	virtual void CreateLast();

	virtual void Union(ECBEntity*);

	virtual bool CanCreate(ECBEntity*) { return false; }

/* Attributs */
public:

/* Variables priv�es */
protected:
	ECBCase* CheckMove(uint mx, uint my);
};

#endif /* ECLIB_UNITS_H */
