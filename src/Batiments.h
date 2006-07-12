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

class Color;

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

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual void RefreshColor(Color last);

private:
	ECSpriteBase *img;
};

/********************************************************************************************
 *                                ECNuclearSearch                                           *
 ********************************************************************************************/

class ECNuclearSearch : public ECBatiment, public ECBNuclearSearch
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECNuclearSearch) {}

	ENTITY_CONSTRUCTOR(ECNuclearSearch), ECBatiment(Resources::NuclearSearch_Face()) {}

	virtual void Init();

/* Infos */
public:

	virtual const char* Name() const { return "Centre de recherches nucléaire"; }
	virtual const char* Infos() const { return "Fabrication de missiles nucléaires"; }
	virtual ECImage* Icon() const { return Resources::NuclearSearch_Icon(); }

/* Methodes */
public:

	virtual void RecvData(ECData);

	virtual std::string SpecialInfo();
};

/********************************************************************************************
 *                                         ECSilo                                           *
 ********************************************************************************************/
#define SILO_MISSILE_STEP 3
class ECSilo : public ECBatiment, public ECBSilo
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECSilo) : missile(this, SILO_MISSILE_STEP) {}

	ENTITY_CONSTRUCTOR(ECSilo), ECBatiment(Resources::Silo_Face()), missile(this, SILO_MISSILE_STEP)
	{
		missile.SetMissileUp(Resources::Silo_Missile_Up());
		missile.SetMissileDown(Resources::Silo_Missile_Down());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Silo de lancement"; }
	virtual const char* Infos() const { return "Silo de lancement de missile nucléaire"; }
	virtual ECImage* Icon() const { return Resources::Silo_Icon(); }

/* Méthodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual void Draw();

	virtual std::string SpecialInfo();

/* Variables privées */
private:
	ECMissile missile;
};

/********************************************************************************************
 *                                ECapitale                                                 *
 ********************************************************************************************/

class ECapitale : public ECBatiment, public ECBCapitale
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECapitale) {}

	ENTITY_CONSTRUCTOR(ECapitale), ECBatiment(Resources::Capitale_Face()) {}

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

	ENTITY_EMPTY_CONSTRUCTOR(ECity) {}

	ENTITY_CONSTRUCTOR(ECity), ECBatiment(Resources::City_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Ville"; }
	virtual const char* Infos() const { return "Rapporte une certaine somme par tours"; }
	virtual ECImage* Icon() const { return Resources::City_Icon(); }
};

/********************************************************************************************
 *                                ECShipyard                                                *
 ********************************************************************************************/

class ECShipyard : public ECBatiment, public ECBShipyard
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECShipyard) {}

	ENTITY_CONSTRUCTOR(ECShipyard), ECBatiment(Resources::Shipyard_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Chantier naval"; }
	virtual const char* Infos() const { return "Construit des bateaux"; }
	virtual ECImage* Icon() const { return Resources::Shipyard_Icon(); }
};

/********************************************************************************************
 *                                ECharFact                                                 *
 ********************************************************************************************/

class ECharFact : public ECBatiment, public ECBCharFact
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECharFact) {}

	ENTITY_CONSTRUCTOR(ECharFact), ECBatiment(Resources::CharFact_Face()) {}

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

	ENTITY_EMPTY_CONSTRUCTOR(ECaserne) {}

	ENTITY_CONSTRUCTOR(ECaserne), ECBatiment(Resources::Caserne_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Caserne"; }
	virtual const char* Infos() const { return "Caserne pour entraîner des hommes"; }
	virtual ECImage* Icon() const { return Resources::Caserne_Icon(); }
};

#endif /* EC_BATIMENTS_H */
