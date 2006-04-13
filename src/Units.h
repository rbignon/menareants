/* src/Units.h - Header of Units.cpp
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

#ifndef EC_UNITS_H
#define EC_UNITS_H

#include "src/Map.h"
#include "lib/Units.h"
#include "Resources.h"

/********************************************************************************************
 *                                ECArmy                                                    *
 ********************************************************************************************/

class ECArmy : public ECEntity, public ECBArmy
{
/* Constructeur/Destructeur */
public:

	ECArmy() : ECBEntity(E_ARMY) {}

	ECArmy(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = ARMY_NB)
		: ECBEntity(_name, _owner, _case, E_ARMY, ARMY_STEP, ARMY_COST, _nb)
	{ SetImage(Resources::Army_Face()); }


/* Infos */
public:

	virtual const char* Name() const { return "Armée"; }
	virtual const char* Infos() const { return "Armée de base"; }
	virtual ECImage* Icon() const { return Resources::Army_Icon(); }

/* Methodes */
public:

	virtual bool BeforeEvent();

	virtual bool MakeEvent();

	virtual bool AfterEvent();

/* Attributs */
public:

/* Variables protégées */
protected:
};

#endif /* EC_UNITS_H */
