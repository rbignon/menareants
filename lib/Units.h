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
 *                               ECBArmee                                                   *
 ********************************************************************************************/
/** This is a simple army */
class ECBArmee : virtual public ECBEntity
{
/* Constructeur/Destructeur */
public:
	ECBArmee(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb)
		: ECBEntity(_name, _owner, _case, E_ARMEE), nb(_nb)
	{}

/* Methodes */
public:

/* Attributs */
public:

	/** Return the number of soldats in the army */
	uint Nb() { return nb; }

/* Variables privées */
protected:
	uint nb;
};

#endif /* ECLIB_UNITS_H */
