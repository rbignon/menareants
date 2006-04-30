/* server/Batiments.h - Header of Batiments.cpp
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

#ifndef ECD_BATIMENTS_H
#define ECD_BATIMENTS_H

#include "server/Map.h"
#include "lib/Batiments.h"

/********************************************************************************************
 *                               ECharFact                                                  *
 ********************************************************************************************/
class ECharFact : public ECEntity, public ECBCharFact
{
/* Constructeur/Destructeur */
public:

	CHARFACT_EMPTY_CONSTRUCTOR(ECharFact) {}

	CHARFACT_CONSTRUCTOR(ECharFact) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void CreateLast() {}

	virtual void Union(ECEntity*) { return; }

/* Attributs */
public:

/* Variables protégées */
protected:

};

/********************************************************************************************
 *                               ECaserne                                                   *
 ********************************************************************************************/
class ECaserne : public ECEntity, public ECBCaserne
{
/* Constructeur/Destructeur */
public:

	CASERNE_EMPTY_CONSTRUCTOR(ECaserne) {}

	CASERNE_CONSTRUCTOR(ECaserne) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void CreateLast() {}

	virtual void Union(ECEntity*) { return; }

/* Attributs */
public:

/* Variables protégées */
protected:

};

#endif /* ECD_BATIMENTS_H */
