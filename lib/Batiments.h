/* lib/Batiments.h - Header of Batiments.cpp
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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

/********************************************************************
 *
 *            DEAR CHEATERS
 *
 * It's useless to change this values in your client, because this
 * is the server who decides if an action is accepted or not.
 * For example, if you change a price, your client will show you
 * this new price, but the server will use the REAL price!
 * Idem for Initial Number, Visibility, Porty, etc.
 *
 * Regards.
 ********************************************************************/

#include "Map.h"
#include "Channels.h"

/********************************************************************************************
 *                                        ECBarbedWire                                      *
 ********************************************************************************************/
/** This is a barbedwire. */
class ECBBarbedWire : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBBarbedWire) {}

	ENTITY_CONSTRUCTOR(ECBBarbedWire) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_BARBEDWIRE; }
	virtual uint Cost() const { return 1000; }
	virtual uint InitNb() const { return 1;}
	virtual uint Visibility() const { return 0; } /**< Il ne voit rien du tout */

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	/** BarbedWire is a building to prevent from constructing buildings here, and to be drawed at background. */
	virtual bool IsBuilding() const { return true; }
	virtual bool IsTerrain() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }
};

/********************************************************************************************
 *                                        ECBTrees                                          *
 ********************************************************************************************/
/** This is Forest */
class ECBTrees : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBTrees) {}

	ENTITY_CONSTRUCTOR(ECBTrees) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_TREES; }
	virtual uint Cost() const { return 1000; }
	virtual uint InitNb() const { return 1;}
	virtual uint Visibility() const { return 0; } /**< Elle ne voit rien du tout */

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	/** Trees is a building to prevent from constructing buildings here, and to be drawed at background. */
	virtual bool IsBuilding() const { return true; }
	virtual bool IsTerrain() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }
};

/********************************************************************************************
 *                                        ECBMine                                           *
 ********************************************************************************************/
#define MINE_CHARGETIME             2
/** This is a mine */
class ECBMine : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBMine) : restBuild(MINE_CHARGETIME) {}

	ENTITY_CONSTRUCTOR(ECBMine), restBuild(MINE_CHARGETIME) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_MINE; }
	virtual uint Cost() const { return 3000; }
	virtual uint InitNb() const { return 1;}
	virtual uint Visibility() const { return 1; } /**< Elle ne voit en effet pas bien loins... */
	virtual bool IsTerrain() const { return true; } /**< C'est pour qu'on puisse en construire sur les arbres */

	enum data_t {
		DATA_RESTBUILD
	};

	virtual bool CanAttaq(const ECBEntity* e) { return (!restBuild && e->Level() <= L_GROUND); }

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

/* Variables privées */
protected:
	uint restBuild;
};

/********************************************************************************************
 *                               ECBEiffelTower                                             *
 ********************************************************************************************/
/** This is a radar */
class ECBEiffelTower : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBEiffelTower) {}

	ENTITY_CONSTRUCTOR(ECBEiffelTower) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_EIFFELTOWER; }
	virtual uint Cost() const { return 40000; }
	virtual uint InitNb() const { return 3000;}

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }

	virtual bool CanBeCreated(ECBPlayer* pl) const;
};


/********************************************************************************************
 *                               ECBRadar                                                   *
 ********************************************************************************************/
/** This is a radar */
class ECBRadar : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBRadar) {}

	ENTITY_CONSTRUCTOR(ECBRadar) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_RADAR; }
	virtual uint Cost() const { return 20000; }
	virtual uint InitNb() const { return 1000;}

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }

	virtual bool CanBeCreated(ECBPlayer* pl) const;
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

	void Init();

/* Constantes */
public:

	virtual e_type Type() const { return E_NUCLEARSEARCH; }
	virtual uint Cost() const { return 20000; }
	virtual uint InitNb() const { return 3000;}

	enum data_t {
		DATA_NBMISSILES,
		DATA_RESTBUILD
	};

	virtual bool CanAttaq(const ECBEntity* e) { return e->Level() == Level(); }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const;
	virtual void SetOwner(ECBPlayer* _p);

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
#define SILO_IMPACT                1
#define SILO_CHARGETIME            2
/** This is a char factory */
class ECBSilo : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBSilo) : nuclear_search(0), restBuild(SILO_CHARGETIME) {}

	ENTITY_CONSTRUCTOR(ECBSilo), nuclear_search(0), restBuild(SILO_CHARGETIME) {}

	virtual ~ECBSilo() {}

/* Constantes */
public:

	virtual e_type Type() const { return E_SILO; }
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 500;}
	virtual uint Porty() const { return 15; }

	enum data_t {
		DATA_RESTBUILD
	};

	virtual bool CanAttaq(const ECBEntity* e)
	{
		return (e->Level() <= L_GROUND && !e->IsCountryMaker());
	}

	virtual void Init();

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return (!restBuild && NuclearSearch() && NuclearSearch()->Missiles()); }

	virtual bool CanCreate(const ECBEntity* e) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const;
	virtual void SetOwner(ECBPlayer* _p);

/* Attributs */
public:

	ECBNuclearSearch* NuclearSearch() const { return nuclear_search; }
	void SetNuclearSearch(ECBNuclearSearch* ns) { nuclear_search = ns; }

	uint RestBuild() const { return restBuild; }

/* Variables privées */
private:
	ECBNuclearSearch* nuclear_search;

protected:
	uint restBuild;
};

/********************************************************************************************
 *                               ECBGulag                                                   *
 ********************************************************************************************/
class ECBGulag : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBGulag) : nb_prisoners(0) {}

	ENTITY_CONSTRUCTOR(ECBGulag), nb_prisoners(0) {}

/* Constantes */
public:

	enum data_t {
		DATA_NB_PRISONERS
	};

	virtual e_type Type() const { return E_GULAG; }
	virtual uint Cost() const { return 30000; }
	virtual uint InitNb() const { return 200; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_URSS); }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }
	virtual bool CanCreate(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

	int TurnMoney(ECBPlayer* pl) { return (pl == Owner()) ? nb_prisoners/2 : 0; }

	uint NbPrisoners() const { return nb_prisoners; }
	uint MaxPrisoners() const { return 10000; }

protected:
	uint nb_prisoners;
};

/********************************************************************************************
 *                               ECBMegalopole                                              *
 ********************************************************************************************/
/** This is a megalopole */
class ECBMegalopole : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBMegalopole) {}

	ENTITY_CONSTRUCTOR(ECBMegalopole) {}

	virtual ~ECBMegalopole() {}

/* Constantes */
public:

	virtual e_type Type() const { return E_MEGALOPOLE; }
	virtual uint Cost() const { return 50000; }
	virtual uint InitNb() const { return 5000; }
	virtual uint Visibility() const { return 5; }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_ARMY:
			case ECBEntity::E_CHAR:
				return true;
			default:
				return false;
		}
	}

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() const { return true; }
	virtual bool IsCity() const { return true; }
	virtual int TurnMoney(ECBPlayer* pl) { return (pl == Owner()) ? (5 * Map()->CityMoney()) : 0; }
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
	virtual uint Cost() const { return 15000; }
	virtual uint InitNb() const { return 2000; }
	virtual uint Visibility() const { return 4; }
	virtual e_type MyUpgrade() const { return E_MEGALOPOLE; }

	virtual bool CanCreate(const ECBEntity* e) { return (e->Type() == ECBEntity::E_ARMY); }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() const { return true; }
	virtual bool IsCity() const { return true; }
	virtual int TurnMoney(ECBPlayer* pl) { return (pl == Owner()) ? (2 * Map()->CityMoney()) : 0; }
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

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }
	virtual bool IsCountryMaker() const { return true; }
	virtual bool IsCity() const { return true; }
	virtual int TurnMoney(ECBPlayer* pl) { return (pl == Owner()) ? Map()->CityMoney() : 0; }
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
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 1000; }
	virtual uint Visibility() const { return 4; }
	virtual uint Porty() const { return 3; }
	virtual e_type MyUpgrade() const { return E_OBELISK; }

	virtual bool CanAttaq(const ECBEntity* e)
	{
		if(!e->IsBuilding() && e->Case() != Case() && e->Level() <= L_GROUND) return true;
		else return false;
	}

	bool CanCreate(const ECBEntity*) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return true; }
	virtual bool CanBeCreated(ECBPlayer* pl) const { return true; }
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
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 2000; }
	virtual uint Visibility() const { return 4; }
	virtual uint Porty() const { return 5; }

	virtual bool CanAttaq(const ECBEntity* e)
	{
		if(!e->IsCountryMaker() && e->Case() != Case()) return true;
		else return false;
	}

	bool CanCreate(const ECBEntity*) { return false; }

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
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 1000; }
	virtual bool IsCity() const { return true; }

	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

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
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 500; }
	virtual bool IsCity() const { return true; }

	virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

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
			case E_JOUANO:
				return true;
			default:
				return false;
		}
	}
};

/********************************************************************************************
 *                               ECBAirPort                                                 *
 ********************************************************************************************/
/** This is an airport */
class ECBAirPort : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBAirPort) {}

	ENTITY_CONSTRUCTOR(ECBAirPort) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_AIRPORT; }
	virtual uint Cost() const { return 1000; }
	virtual uint InitNb() const { return 500; }
	virtual bool IsCity() const { return true; }

	/* We DON'T allow someone to sell his airport, to prevent of selling when an enemy plane come on */

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_PLANE:
			case ECBEntity::E_BOEING:
				return true;
			default:
				return false;
		}
	}
};

/********************************************************************************************
 *                               ECBShipyard                                                *
 ********************************************************************************************/
class ECBShipyard : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBShipyard) {}

	ENTITY_CONSTRUCTOR(ECBShipyard) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_SHIPYARD; }
	virtual uint Cost() const { return 2000; }
	virtual uint InitNb() const { return 200; }

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

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

/********************************************************************************************
 *                               ECBCavern                                                  *
 ********************************************************************************************/
class ECBCavern : public virtual ECBContainer
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBCavern) {}

	ENTITY_CONSTRUCTOR(ECBCavern) {}

	virtual ~ECBCavern();

	void Init();

/* Constantes */
public:

	virtual e_type Type() const { return E_CAVERN; }
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 200; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_NOISY); }

	virtual uint UnitaryCapacity() const { return 10000; }

	virtual bool CanContainThisEntity(const ECBEntity* et) const
	{
		return (et->IsInfantry() || et->IsVehicule());
	}

	virtual bool Contain(ECBEntity* e);
	virtual bool UnContain();

	virtual bool CanAttaq(const ECBEntity* e) { return false; }

	virtual bool IsBuilding() const { return true; }
	virtual bool IsOnMontain() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return false; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool CanCreate(const ECBEntity* e) { return false; }
	virtual void SetOwner(ECBPlayer* _p);

protected:
	/* Set this contained entity on every caverns. */
	void BroadcastContaining(ECBEntity* e);
};

#endif /* ECLIB_BATIMENTS_H */
