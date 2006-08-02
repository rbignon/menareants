/* lib/Batiments.h - Header of Batiments.cpp
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

#ifndef ECLIB_BATIMENTS_H
#define ECLIB_BATIMENTS_H

#include "Map.h"

/********************************************************************************************
 *                                        ECBMine                                           *
 ********************************************************************************************/
#define MINE_CHARGETIME             2
/** This is a char factory */
class ECBMine : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBMine) : restBuild(MINE_CHARGETIME) {}

	ENTITY_CONSTRUCTOR(ECBMine), restBuild(MINE_CHARGETIME) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_MINE; }
	virtual uint Cost() const { return 15000; }
	virtual uint InitNb() const { return 1;}
	virtual uint Visibility() const { return 1; } /**< Elle ne voit en effet pas bien loins... */

	enum data_t {
		DATA_RESTBUILD
	};

	virtual bool CanAttaq(const ECBEntity* e) { return (!restBuild); }

	virtual const char* Qual() const { return "la mine"; }

	/** Mine is a building to prevent from constructing buildings here, and to be drawed at background. */
	virtual bool IsBuilding() const { return true; }
	virtual bool IsHidden() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }

/* Attributs */
public:

	uint RestBuild() const { return restBuild; }

/* Variables priv�es */
protected:
	uint restBuild;
};

/********************************************************************************************
 *                               ECBNuclearSearch                                           *
 ********************************************************************************************/
#define NUCLEARSEARCH_BUILDTIME             10
#define NUCLEARSEARCH_INITBUILDTIME         5
/** This is a nuclear search center */
class ECBNuclearSearch : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBNuclearSearch) : missiles(0), restBuild(NUCLEARSEARCH_INITBUILDTIME) {}

	ENTITY_CONSTRUCTOR(ECBNuclearSearch), missiles(0), restBuild(NUCLEARSEARCH_INITBUILDTIME) {}

	virtual ~ECBNuclearSearch();

/* Constantes */
public:

	virtual e_type Type() const { return E_NUCLEARSEARCH; }
	virtual uint Cost() const { return 50000; }
	virtual uint InitNb() const { return 3000;}

	enum data_t {
		DATA_NBMISSILES,
		DATA_RESTBUILD
	};

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual const char* Qual() const { return "le centre de recherches nucl�aire"; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const;

/* Attributs */
public:

	uint& Missiles() { return missiles; }
	uint RestBuild() const { return restBuild; }

/* Variables priv�es */
protected:
	uint missiles, restBuild;
};

/********************************************************************************************
 *                               ECBSilo                                                    *
 ********************************************************************************************/
#define SILO_IMPACT                1
/** This is a char factory */
class ECBSilo : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBSilo) : nuclear_search(0) {}

	ENTITY_CONSTRUCTOR(ECBSilo), nuclear_search(0) {}

	virtual ~ECBSilo() {}

/* Constantes */
public:

	virtual e_type Type() const { return E_SILO; }
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 500;}
	virtual uint Porty() const { return 10; }

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual void Init();

	virtual const char* Qual() const { return "le silo de lancement"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return true; }

	virtual bool CanCreate(const ECBEntity* e) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const;

/* Attributs */
public:

	ECBNuclearSearch* NuclearSearch() const { return nuclear_search; }
	void ClearNuclearSearch() { nuclear_search = 0; }

/* Variables priv�es */
private:
	ECBNuclearSearch* nuclear_search;
};

/********************************************************************************************
 *                               ECBCapitale                                                *
 ********************************************************************************************/
/** This is a city */
class ECBCapitale : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBCapitale) {}

	ENTITY_CONSTRUCTOR(ECBCapitale) {}

	virtual ~ECBCapitale() {}

/* Constantes */
public:

	virtual e_type Type() const { return E_CAPITALE; }
	virtual uint Cost() const { return 50000; }
	virtual uint InitNb() const { return 2000; }
	virtual uint Visibility() const { return 4; }

	bool CanCreate(const ECBEntity*) { return false; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual const char* Qual() const { return "le centre d'affaire"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() const { return true; }
	virtual bool IsCity() const { return true; }
	virtual int TurnMoney() { return Case() ? (2 * Case()->Map()->CityMoney()) : 0; }
};

/********************************************************************************************
 *                               ECBCity                                                    *
 ********************************************************************************************/
/** This is a city */
class ECBCity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBCity) {}

	ENTITY_CONSTRUCTOR(ECBCity) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_CITY; }
	virtual uint Cost() const { return 0; }
	virtual uint InitNb() const { return 1000; }
	virtual e_type MyUpgrade() const { return E_CAPITALE; }

	bool CanCreate(const ECBEntity*) { return false; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual const char* Qual() const { return "le centre ville"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() const { return true; }
	virtual bool IsCity() const { return true; }
	virtual int TurnMoney() { return Case() ? Case()->Map()->CityMoney() : 0; }
};

/********************************************************************************************
 *                               ECBDefenseTower                                            *
 ********************************************************************************************/
/** This is a defense tower */
class ECBDefenseTower : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBDefenseTower) {}

	ENTITY_CONSTRUCTOR(ECBDefenseTower) {}

	virtual ~ECBDefenseTower() {}

/* Constantes */
public:

	virtual e_type Type() const { return E_DEFENSETOWER; }
	virtual uint Cost() const { return 0; }
	virtual uint InitNb() const { return 1000; }
	virtual uint Visibility() const { return 4; }
	virtual uint Porty() const { return 3; }
	virtual e_type MyUpgrade() const { return E_OBELISK; }

	virtual bool CanAttaq(const ECBEntity* e)
	{
		if(!e->IsBuilding() && e->Case() != Case()) return true;
		else return false;
	}

	bool CanCreate(const ECBEntity*) { return false; }

	virtual const char* Qual() const { return "la tour de d�fense"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return true; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCity() const { return true; }
};

/********************************************************************************************
 *                               ECBObelisk                                                 *
 ********************************************************************************************/
/** This is an obelisk */
class ECBObelisk : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBObelisk) {}

	ENTITY_CONSTRUCTOR(ECBObelisk) {}

	virtual ~ECBObelisk() {}

/* Constantes */
public:

	virtual e_type Type() const { return E_OBELISK; }
	virtual uint Cost() const { return 20000; }
	virtual uint InitNb() const { return 2000; }
	virtual uint Visibility() const { return 4; }
	virtual uint Porty() const { return 5; }

	virtual bool CanAttaq(const ECBEntity* e)
	{
		if(!e->IsCountryMaker() && e->Case() != Case()) return true;
		else return false;
	}

	bool CanCreate(const ECBEntity*) { return false; }

	virtual const char* Qual() const { return "l'ob�lisque"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return true; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCity() const { return true; }
};

/********************************************************************************************
 *                               ECBCharFact                                                *
 ********************************************************************************************/
/** This is a char factory */
class ECBCharFact : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBCharFact) {}

	ENTITY_CONSTRUCTOR(ECBCharFact) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_CHARFACT; }
	virtual uint Cost() const { return 20000; }
	virtual uint InitNb() const { return 1000; }
	virtual bool IsCity() const { return true; }

	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual const char* Qual() const { return "l'usine de chars"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_CHAR:
			case ECBEntity::E_MISSILAUNCHER:
				return true;
			default:
				return false;
		}
	}
};

/********************************************************************************************
 *                               ECBCaserne                                                 *
 ********************************************************************************************/
/** This is a caserne */
class ECBCaserne : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBCaserne) {}

	ENTITY_CONSTRUCTOR(ECBCaserne) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_CASERNE; }
	virtual uint Cost() const { return 9000; }
	virtual uint InitNb() const { return 500; }
	virtual bool IsCity() const { return true; }

	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual const char* Qual() const { return "la caserne"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
			case E_ENGINER:
			case E_TOURIST:
			case E_MCDO:
				return true;
			default:
				return false;
		}
	}
};

/********************************************************************************************
 *                               ECBShipyard                                                *
 ********************************************************************************************/
/** This is a char factory */
class ECBShipyard : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBShipyard) {}

	ENTITY_CONSTRUCTOR(ECBShipyard) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_SHIPYARD; }
	virtual uint Cost() const { return 4000; }
	virtual uint InitNb() const { return 200; }

	virtual bool CanAttaq(const ECBEntity* e) { return !e->CanInvest(this); }

	virtual const char* Qual() const { return "le port"; }
	virtual bool IsBuilding() const { return true; }
	virtual bool IsNaval() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_BOAT:
				return true;
			default:
				return false;
		}
	}
};


#endif /* ECLIB_BATIMENTS_H */
