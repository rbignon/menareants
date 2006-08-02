/* lib/Units.h - Header of Units.cpp
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

#ifndef ECLIB_UNITS_H
#define ECLIB_UNITS_H

#include "Map.h"
#include "Channels.h"

/********************************************************************************************
 *                               ECBContainer                                               *
 ********************************************************************************************/
class ECBContainer : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:
	ECBContainer()
		: unit(0)
	{}

	virtual ~ECBContainer();

/* Methodes */
public:

	bool Contain(ECBEntity*);
	bool UnContain();

/* Attributs */
public:

	ECBEntity* Containing() const { return unit; }
	void SetContaining(ECBEntity* e) { unit = e; }

	virtual uint RealNb() const { return unit ? (unit->Nb() + Nb()) : Nb(); }

/* Variables privées */
private:
	ECBEntity* unit;
};

/********************************************************************************************
 *                               ECBBoat                                                    *
 ********************************************************************************************/
/** This is a boat. */
class ECBBoat : public virtual ECBContainer
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBBoat) {}

	ENTITY_CONSTRUCTOR(ECBBoat) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_BOAT; }
	virtual uint Cost() const { return 2000; }
	virtual uint InitNb() const { return 10; }
	virtual uint Step() const { return 4; }
	virtual uint Visibility() const { return 4; }

	virtual bool CanContain(const ECBEntity* et)
	{
		if(et->Nb() > 50*Nb() || !et->IsInfantry())
			return false;
		else
			return true;
	}

	bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_BOAT:
			case E_SHIPYARD:
				return true;
			default:
				return false;
		}
	}
	virtual const char* Qual() const { return "le bateau"; }
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool IsNaval() const { return true; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

/********************************************************************************************
 *                               ECBMissiLauncher                                           *
 ********************************************************************************************/
/** This is a missile launcher */
class ECBMissiLauncher : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBMissiLauncher) {}

	ENTITY_CONSTRUCTOR(ECBMissiLauncher) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_MISSILAUNCHER; }
	virtual uint Cost() const { return 6000; }
	virtual uint InitNb() const { return 100; }
	virtual uint Step() const { return 1; }
	virtual uint Porty() const { return 8; }

	bool CanAttaq(const ECBEntity* e)
	{
		if(e->Case() == Case() || e->IsCountryMaker() ||
		   !e->IsVehicule() && !e->IsInfantry() && !e->IsBuilding() && !e->IsNaval())
			return false;
		else
			return true;
	}
	virtual const char* Qual() const { return "le lance-missiles"; }
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool WantDeploy() { return !(EventType() & ARM_ATTAQ); } ///< Default = false
	virtual bool WantAttaq(uint, uint, bool) { return Deployed(); }
	bool IsVehicule() const { return true; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

/********************************************************************************************
 *                               ECBChar                                                    *
 ********************************************************************************************/
/** This is a vehicule */
class ECBChar : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBChar) {}

	ENTITY_CONSTRUCTOR(ECBChar) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_CHAR; }
	virtual uint Cost() const { return 20000; }
	virtual uint InitNb() const { return 1500; }
	virtual uint Step() const { return 3; }

	bool CanAttaq(const ECBEntity* e)
	{
		if((Parent() && Parent()->CanAttaq(e)) ||
		   ((e->IsInfantry() || e->IsVehicule() || e->IsBuilding() && !e->IsCity() && !e->IsHidden()) && !e->IsNaval()))
			return true;
		else
			return false;
	}
	virtual const char* Qual() const { return "le char"; }
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsVehicule() const { return true; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

/********************************************************************************************
 *                               ECBMcDo                                                    *
 ********************************************************************************************/
/** This is a special Unit for USA nation */
class ECBMcDo : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBMcDo) {}

	ENTITY_CONSTRUCTOR(ECBMcDo) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_MCDO; }
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 1; }
	virtual uint Step() const { return 2; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_USA); }

	bool CanInvest(const ECBEntity* e) const
	{
		if(e->Type() == E_CASERNE && !Like(e))
			return true;
		else
			return false;
	}
	bool CanAttaq(const ECBEntity* e) { return false; }
	virtual const char* Qual() const { return "Donald de McGerbale"; }
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsInfantry() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool AddUnits(uint) { return false; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

/********************************************************************************************
 *                               ECBTourist                                                 *
 ********************************************************************************************/
/** This is a special Unit for Japan nation */
class ECBTourist : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBTourist) {}

	ENTITY_CONSTRUCTOR(ECBTourist) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_TOURIST; }
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 1; }
	virtual uint Step() const { return 4; }
	virtual uint Visibility() const { return 4; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_JAPON); }

	bool CanInvest(const ECBEntity* e) const { return false; }
	bool CanAttaq(const ECBEntity* e) { return false; }
	virtual const char* Qual() const { return "le touriste japonais"; }
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsInfantry() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool AddUnits(uint) { return false; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};


/********************************************************************************************
 *                               ECBEnginer                                                 *
 ********************************************************************************************/
/** This is an enginer who can take enemey's buildings */
class ECBEnginer : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBEnginer) {}

	ENTITY_CONSTRUCTOR(ECBEnginer) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_ENGINER; }
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 1; }
	virtual uint Step() const { return 1; }

	virtual bool CanInvest(const ECBEntity* e) const
	{
		if(e->IsBuilding() && !e->IsNaval() && (!Like(e) || e->Owner() == Owner() && e->Nb() < e->InitNb()))
			return true;
		else
			return false;
	}

	bool CanAttaq(const ECBEntity* e) { return false; }
	virtual const char* Qual() const { return "l'ingénieur"; }
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsInfantry() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool AddUnits(uint) { return false; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};


/********************************************************************************************
 *                               ECBArmy                                                   *
 ********************************************************************************************/
/** This is a simple army */
class ECBArmy : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBArmy) {}

	ENTITY_CONSTRUCTOR(ECBArmy) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_ARMY; }
	virtual uint Cost() const { return 2000; }
	virtual uint InitNb() const { return 100; }
	virtual uint Step() const { return 2; }

	bool CanAttaq(const ECBEntity* e)
	{
		if((Parent() && Parent()->CanAttaq(e)) ||
		   (( e->IsInfantry() && e->Type() != E_MCDO ||
		      e->IsVehicule() ||
		      e->IsBuilding() && !e->IsCity() && !e->IsHidden()) &&
		    !e->IsNaval()))
			return true;
		else
			return false;
	}
	virtual const char* Qual() const { return "l'armée"; }
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsInfantry() const { return true; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

#endif /* ECLIB_UNITS_H */
