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

#ifndef EC_BATIMENTS_H
#define EC_BATIMENTS_H

#include "src/Map.h"
#include "lib/Batiments.h"
#include "Resources.h"

/********************************************************************************************
 *                                ECharFact                                                     *
 ********************************************************************************************/

class ECharFact : public ECEntity, public ECBCharFact
{
/* Constructeur/Destructeur */
public:

	CHARFACT_EMPTY_CONSTRUCTOR(ECharFact) {}

	CHARFACT_CONSTRUCTOR(ECharFact) { SetImage(Resources::CharFact_Face()); }

/* Infos */
public:

	virtual const char* Name() const { return "Usine de chars"; }
	virtual const char* Infos() const { return "Construit les véhicules de votre armée."; }
	virtual ECImage* Icon() const { return Resources::CharFact_Icon(); }

/* Methodes */
public:

	virtual bool BeforeEvent() { return true; }

	virtual bool MakeEvent() { return true; }

	virtual bool AfterEvent() { return true; }

/* Attributs */
public:

/* Variables protégées */
protected:
};

/********************************************************************************************
 *                                ECaserne                                                  *
 ********************************************************************************************/

class ECaserne : public ECEntity, public ECBCaserne
{
/* Constructeur/Destructeur */
public:

	CASERNE_EMPTY_CONSTRUCTOR(ECaserne) {}

	CASERNE_CONSTRUCTOR(ECaserne) { SetImage(Resources::Caserne_Face()); }

/* Infos */
public:

	virtual const char* Name() const { return "Caserne"; }
	virtual const char* Infos() const { return "Caserne pour entraîner des hommes"; }
	virtual ECImage* Icon() const { return Resources::Caserne_Icon(); }

/* Methodes */
public:

	virtual bool BeforeEvent() { return true; }

	virtual bool MakeEvent() { return true; }

	virtual bool AfterEvent() { return true; }

/* Attributs */
public:

/* Variables protégées */
protected:
};

#endif /* EC_BATIMENTS_H */
