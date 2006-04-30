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

/********************************************************************************************
 *                               EChar                                                      *
 ********************************************************************************************/
class EChar : public ECEntity, public ECBChar
{
/* Constructeur/Destructeur */
public:

	CHAR_EMPTY_CONSTRUCTOR(EChar) {}

	CHAR_CONSTRUCTOR(EChar) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool WantMove(ECBMove::E_Move);

	virtual bool WantAttaq(uint x, uint y);

	virtual void CreateLast();

/* Attributs */
public:

/* Variables protégées */
protected:

};

/********************************************************************************************
 *                               ECArmy                                                     *
 ********************************************************************************************/
class ECArmy : public ECEntity, public ECBArmy
{
/* Constructeur/Destructeur */
public:

	ARMY_EMPTY_CONSTRUCTOR(ECArmy) {}

	ARMY_CONSTRUCTOR(ECArmy) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool WantMove(ECBMove::E_Move);

	virtual bool WantAttaq(uint x, uint y);

	virtual void CreateLast();

/* Attributs */
public:

/* Variables protégées */
protected:

};

#endif /* ECD_UNITS_H */
