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
 *                                ECBatiment                                                *
 ********************************************************************************************/

class ECBatiment : public ECEntity
{
/* Constructeur/Destructeur */
public:

	ECBatiment() : img(0) {}

	ECBatiment(ECSpriteBase* b);
	virtual ~ECBatiment();

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&) { return true; }

	virtual bool MakeEvent(const std::vector<ECEntity*>&) { return true; }

	virtual bool AfterEvent(const std::vector<ECEntity*>&) { return true; }

	virtual void RefreshColor(SDL_Color last);

private:
	ECSpriteBase *img;
};

/********************************************************************************************
 *                                ECapitale                                                 *
 ********************************************************************************************/

class ECapitale : public ECBatiment, public ECBCapitale
{
/* Constructeur/Destructeur */
public:

	CAPITALE_EMPTY_CONSTRUCTOR(ECapitale) {}

	CAPITALE_CONSTRUCTOR(ECapitale), ECBatiment(Resources::Capitale_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Ville"; }
	virtual const char* Infos() const { return "Rapporte deux fois plus d'argent qu'une ville"; }
	virtual ECImage* Icon() const { return Resources::Capitale_Icon(); }
};

/********************************************************************************************
 *                                ECity                                                     *
 ********************************************************************************************/

class ECity : public ECBatiment, public ECBCity
{
/* Constructeur/Destructeur */
public:

	CITY_EMPTY_CONSTRUCTOR(ECity) {}

	CITY_CONSTRUCTOR(ECity), ECBatiment(Resources::City_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Ville"; }
	virtual const char* Infos() const { return "Rapporte une certaine somme par tours"; }
	virtual ECImage* Icon() const { return Resources::City_Icon(); }
};

/********************************************************************************************
 *                                ECharFact                                                 *
 ********************************************************************************************/

class ECharFact : public ECBatiment, public ECBCharFact
{
/* Constructeur/Destructeur */
public:

	CHARFACT_EMPTY_CONSTRUCTOR(ECharFact) {}

	CHARFACT_CONSTRUCTOR(ECharFact), ECBatiment(Resources::CharFact_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Usine de chars"; }
	virtual const char* Infos() const { return "Construit les véhicules de votre armée."; }
	virtual ECImage* Icon() const { return Resources::CharFact_Icon(); }
};

/********************************************************************************************
 *                                ECaserne                                                  *
 ********************************************************************************************/

class ECaserne : public ECBatiment, public ECBCaserne
{
/* Constructeur/Destructeur */
public:

	CASERNE_EMPTY_CONSTRUCTOR(ECaserne) {}

	CASERNE_CONSTRUCTOR(ECaserne), ECBatiment(Resources::Caserne_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Caserne"; }
	virtual const char* Infos() const { return "Caserne pour entraîner des hommes"; }
	virtual ECImage* Icon() const { return Resources::Caserne_Icon(); }
};

#endif /* EC_BATIMENTS_H */
