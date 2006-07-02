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
 *                               ECNuclearSearch                                            *
 ********************************************************************************************/
class ECNuclearSearch : public ECEntity, public ECBNuclearSearch
{
/* Constructeur/Destructeur */
public:

	NUCLEARSEARCH_EMPTY_CONSTRUCTOR(ECNuclearSearch) {}

	NUCLEARSEARCH_CONSTRUCTOR(ECNuclearSearch) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void CreateLast() {}

	virtual void Union(ECEntity*) { return; }

	virtual void Played();

	void RemoveOneMissile();
};

/********************************************************************************************
 *                                 ECSilo                                                   *
 ********************************************************************************************/
class ECSilo : public ECEntity, public ECBSilo
{
/* Constructeur/Destructeur */
public:

	SILO_EMPTY_CONSTRUCTOR(ECSilo) {}

	SILO_CONSTRUCTOR(ECSilo) {}

	ENTITY_CREATE_LAST(ECSilo);

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void Union(ECEntity*) { return; }

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent*);

	virtual bool WantAttaq(uint x, uint y, bool);
};

/********************************************************************************************
 *                               ECapitale                                                  *
 ********************************************************************************************/
class ECapitale : public ECEntity, public ECBCapitale
{
/* Constructeur/Destructeur */
public:

	CAPITALE_EMPTY_CONSTRUCTOR(ECapitale) {}

	CAPITALE_CONSTRUCTOR(ECapitale) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void CreateLast() {}

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECity                                                      *
 ********************************************************************************************/
class ECity : public ECEntity, public ECBCity
{
/* Constructeur/Destructeur */
public:

	CITY_EMPTY_CONSTRUCTOR(ECity) {}

	CITY_CONSTRUCTOR(ECity) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void CreateLast() {}

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECShipyard                                                 *
 ********************************************************************************************/
class ECShipyard : public ECEntity, public ECBShipyard
{
/* Constructeur/Destructeur */
public:

	SHIPYARD_EMPTY_CONSTRUCTOR(ECShipyard) {}

	SHIPYARD_CONSTRUCTOR(ECShipyard) {}

/* Methodes */
public:

	/** @return last case */
	virtual bool Return() { return false; }

	virtual void CreateLast() {}

	virtual void Union(ECEntity*) { return; }
};

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
};

#endif /* ECD_BATIMENTS_H */
