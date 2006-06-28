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
 *                               ECBNuclearSearch                                           *
 ********************************************************************************************/
#define NUCLEARSEARCH_STEP                  0
#define NUCLEARSEARCH_NB                    3000
#define NUCLEARSEARCH_COST                  50000
#define NUCLEARSEARCH_BUILDTIME             10
#define NUCLEARSEARCH_INITBUILDTIME         5
#define NUCLEARSEARCH_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_NUCLEARSEARCH, NUCLEARSEARCH_COST)
#define NUCLEARSEARCH_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, \
                                              uint _nb = NUCLEARSEARCH_NB) \
                                            :  ECBEntity(_name, _owner, _case, E_NUCLEARSEARCH, NUCLEARSEARCH_STEP, \
                                                         NUCLEARSEARCH_COST, _nb)
/** This is a char factory */
class ECBNuclearSearch : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	NUCLEARSEARCH_EMPTY_CONSTRUCTOR(ECBNuclearSearch), missiles(0), restBuild(NUCLEARSEARCH_INITBUILDTIME) {}

	NUCLEARSEARCH_CONSTRUCTOR(ECBNuclearSearch), missiles(0), restBuild(NUCLEARSEARCH_INITBUILDTIME) {}

	virtual ~ECBNuclearSearch();

/* Constantes */
public:

	enum data_t {
		DATA_NBMISSILES,
		DATA_RESTBUILD
	};

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual const char* Qual() const { return "le centre de recherches nucléaire"; }
	virtual uint InitNb() const { return NUCLEARSEARCH_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const;

/* Attributs */
public:

	uint& Missiles() { return missiles; }
	uint RestBuild() const { return restBuild; }

/* Variables privées */
protected:
	uint missiles, restBuild;
};

/********************************************************************************************
 *                               ECBSilo                                                    *
 ********************************************************************************************/
#define SILO_STEP                  0
#define SILO_NB                    500
#define SILO_COST                  10000
#define SILO_PORTY                 10
#define SILO_IMPACT                1
#define SILO_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_SILO, SILO_COST)
#define SILO_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = SILO_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_SILO, SILO_STEP, SILO_COST, _nb)
/** This is a char factory */
class ECBSilo : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	SILO_EMPTY_CONSTRUCTOR(ECBSilo), nuclear_search(0) {}

	SILO_CONSTRUCTOR(ECBSilo), nuclear_search(0) {}

	virtual ~ECBSilo() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual void Init();

	virtual const char* Qual() const { return "le silo de lancement"; }
	virtual uint InitNb() const { return SILO_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return true; }

	virtual bool CanCreate(const ECBEntity* e) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const;

/* Attributs */
public:

	ECBNuclearSearch* NuclearSearch() const { return nuclear_search; }
	void ClearNuclearSearch() { nuclear_search = 0; }

/* Variables privées */
private:
	ECBNuclearSearch* nuclear_search;
};

/********************************************************************************************
 *                               ECBCapitale                                                *
 ********************************************************************************************/
#define CAPITALE_STEP                  0
#define CAPITALE_NB                    2000
#define CAPITALE_COST                  0
#define CAPITALE_VISIBILITY            4
#define CAPITALE_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CAPITALE, CAPITALE_COST)
#define CAPITALE_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CAPITALE_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_CAPITALE, CAPITALE_STEP, CAPITALE_COST, _nb, \
                                       CAPITALE_VISIBILITY)
/** This is a city */
class ECBCapitale : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CAPITALE_EMPTY_CONSTRUCTOR(ECBCapitale) {}

	CAPITALE_CONSTRUCTOR(ECBCapitale) {}

	virtual ~ECBCapitale() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual const char* Qual() const { return "la capitale"; }
	virtual uint InitNb() const { return CAPITALE_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() { return true; }
	virtual int TurnMoney() { return 2 * Case()->Map()->CityMoney(); }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_ARMY:
				return true;
			default:
				return false;
		}
	}
};

/********************************************************************************************
 *                               ECBCity                                                    *
 ********************************************************************************************/
#define CITY_STEP                  0
#define CITY_NB                    1000
#define CITY_COST                  0
#define CITY_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CITY, CITY_COST)
#define CITY_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CITY_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_CITY, CITY_STEP, CITY_COST, _nb)
/** This is a city */
class ECBCity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CITY_EMPTY_CONSTRUCTOR(ECBCity) {}

	CITY_CONSTRUCTOR(ECBCity) {}

	virtual ~ECBCity() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual const char* Qual() const { return "la ville"; }
	virtual uint InitNb() const { return CITY_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() { return true; }
	virtual int TurnMoney() { return Case()->Map()->CityMoney(); }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_ARMY:
				return true;
			default:
				return false;
		}
	}
};

/********************************************************************************************
 *                               ECBShipyard                                                *
 ********************************************************************************************/
#define SHIPYARD_STEP                  0
#define SHIPYARD_NB                    200
#define SHIPYARD_COST                  4000
#define SHIPYARD_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_SHIPYARD, SHIPYARD_COST)
#define SHIPYARD_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = SHIPYARD_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_SHIPYARD, SHIPYARD_STEP, SHIPYARD_COST, _nb)
/** This is a char factory */
class ECBShipyard : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	SHIPYARD_EMPTY_CONSTRUCTOR(ECBShipyard) {}

	SHIPYARD_CONSTRUCTOR(ECBShipyard) {}

	virtual ~ECBShipyard() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual const char* Qual() const { return "le port"; }
	virtual uint InitNb() const { return SHIPYARD_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool IsNaval() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return false; }

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

/********************************************************************************************
 *                               ECBCharFact                                                *
 ********************************************************************************************/
#define CHARFACT_STEP                  0
#define CHARFACT_NB                    1000
#define CHARFACT_COST                  20000
#define CHARFACT_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CHARFACT, CHARFACT_COST)
#define CHARFACT_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CHARFACT_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_CHARFACT, CHARFACT_STEP, CHARFACT_COST, _nb)
/** This is a char factory */
class ECBCharFact : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CHARFACT_EMPTY_CONSTRUCTOR(ECBCharFact) {}

	CHARFACT_CONSTRUCTOR(ECBCharFact) {}

	virtual ~ECBCharFact() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual const char* Qual() const { return "l'usine de chars"; }
	virtual uint InitNb() const { return CHARFACT_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return false; }

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
#define CASERNE_STEP                  0
#define CASERNE_NB                    500
#define CASERNE_COST                  9000
#define CASERNE_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CASERNE, CASERNE_COST)
#define CASERNE_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CASERNE_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_CASERNE, CASERNE_STEP, CASERNE_COST, _nb)
/** This is a caserne */
class ECBCaserne : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CASERNE_EMPTY_CONSTRUCTOR(ECBCaserne) {}

	CASERNE_CONSTRUCTOR(ECBCaserne) {}

	virtual ~ECBCaserne() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e) { return true; }

	virtual const char* Qual() const { return "la caserne"; }
	virtual uint InitNb() const { return CASERNE_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, bool) { return false; }
	virtual bool WantAttaq(uint x, uint y) { return false; }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_ARMY:
				return true;
			default:
				return false;
		}
	}
};

#endif /* ECLIB_BATIMENTS_H */
