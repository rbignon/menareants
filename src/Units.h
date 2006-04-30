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
 *                                EChar                                                     *
 ********************************************************************************************/

class EChar : public ECEntity, public ECBChar
{
/* Constructeur/Destructeur */
public:

	CHAR_EMPTY_CONSTRUCTOR(EChar) {}

	CHAR_CONSTRUCTOR(EChar) { SetImage(Resources::Char_Face()); }

/* Infos */
public:

	virtual const char* Name() const { return "Char"; }
	virtual const char* Infos() const { return "Char d'assaut"; }
	virtual ECImage* Icon() const { return Resources::Char_Icon(); }

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

/********************************************************************************************
 *                                ECArmy                                                    *
 ********************************************************************************************/

class ECArmy : public ECEntity, public ECBArmy
{
/* Constructeur/Destructeur */
public:

  ARMY_EMPTY_CONSTRUCTOR(ECArmy) {}

	ARMY_CONSTRUCTOR(ECArmy) { SetImage(Resources::Army_Face()); }

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
