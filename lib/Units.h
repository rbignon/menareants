/* lib/Units.h - Header of Units.cpp
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

#ifndef ECLIB_UNITS_H
#define ECLIB_UNITS_H

/********************************************************************
 *
 *             DEAR CHEATERS
 *
 * It's useless to change this values in your client, because this
 * is the server who decide if an action is accepted or not.
 * For example, if you change a price, your client will show you
 * this new price, but the server will use the REAL price!
 * Idem for Initial Number, Visibility, Porty, etc.
 *
 * Regards.
 ********************************************************************/

#include "Map.h"
#include "Channels.h"

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
	virtual uint InitNb() const { return 5; }
	virtual uint Step() const { return 4; }
	virtual uint Visibility() const { return 4; }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_MER)); }
	virtual uint UnitaryCapacity() const { return 100; }

	virtual bool CanContainThisEntity(const ECBEntity* et) const
	{
		if(!et->IsInfantry())
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
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool IsNaval() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

/********************************************************************************************
 *                               ECBPlane                                                   *
 ********************************************************************************************/
class ECBPlane : public virtual ECBContainer
{
public:
	ENTITY_EMPTY_CONSTRUCTOR(ECBPlane) {}

	ENTITY_CONSTRUCTOR(ECBPlane) {}

	virtual void Init() { ECBEntity::Init(); SetDeployed(true); }

	/** Prix par tours de l'avion en vol
	 * Cette fonction constante indique le prix, lorsque l'avion est en vol, que paye
	 * le propriétaire chaque tours où l'avion est en vol, la correspondance entre
	 * le nombre d'hommes contenus et le prix.
	 * Par exemple, si elle retourne \b 1, le joueur payera 1 * nombre_d'hommes $ par tours,
	 * donc 1000 $ par tours si l'avion contient 1000 hommes.
	 */
	virtual uint VolCost() const { return 1; }

	virtual e_type Type() const { return E_PLANE; }
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 10; }
	virtual uint Step() const { return 7; }
	virtual uint Visibility() const { return 5; }
	virtual bool CanWalkOn(ECBCase *c) const { return true; }
	virtual e_level Level() const { return (!Deployed() ^ !!(EventType() & ARM_DEPLOY)) ? L_AIR : L_GROUND; }
	virtual uint UnitaryCapacity() const { return 100; }

	// Cet avion de transport ne peut PAS prendre une ville
	virtual bool CanInvest(const ECBEntity* e) const { return false; }

	virtual bool CanContainThisEntity(const ECBEntity *et) const
	{
		if(!Deployed() ^ !!(EventType() & ARM_DEPLOY) || (!et->IsInfantry() && !et->IsVehicule()) || et->Type() == Type())
			return false;
		return true;
	}

	bool CanAttaq(const ECBEntity *e)
	{
		return false;
	}

	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool IsPlane() const { return true; }
	virtual bool WantDeploy() { return (DestCase()->Entities()->Find(E_AIRPORT).empty() == false); }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

	int TurnMoney(ECBPlayer* pl);
};

/********************************************************************************************
 *                               ECBBoeing                                                  *
 ********************************************************************************************/
class ECBBoeing : public virtual ECBEntity
{
public:
	ENTITY_EMPTY_CONSTRUCTOR(ECBBoeing) {}

	ENTITY_CONSTRUCTOR(ECBBoeing) {}

	virtual void Init() { ECBEntity::Init(); SetDeployed(true); }

	virtual e_type Type() const { return E_BOEING; }
	virtual uint Cost() const { return 10000; }
	virtual uint InitNb() const { return 10; }
	virtual uint Step() const { return 5; }
	virtual uint Visibility() const { return 5; }
	virtual bool CanWalkOn(ECBCase *c) const { return true; }
	virtual e_level Level() const { return (!Deployed() ^ !!(EventType() & ARM_DEPLOY)) ? L_AIR : L_GROUND; }
	virtual uint Porty() const { return 5; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_ALQUAIDA); }

	virtual bool CanInvest(const ECBEntity* e) const { return false; }

	bool CanAttaq(const ECBEntity *e)
	{
		if(e->IsCountryMaker() || e->Case() == Case())
			return false;
		else
			return true;
	}

	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool IsPlane() const { return true; }
	virtual bool WantDeploy() { return (DestCase()->Entities()->Find(E_AIRPORT).empty() == false); }
	virtual bool WantAttaq(uint, uint, bool) { return Level() >= L_AIR; }
	virtual bool WantMove(ECBMove::E_Move, int) { return !(EventType() & ARM_ATTAQ); }
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
	virtual uint Cost() const { return 1000; }
	virtual uint InitNb() const { return 100; }
	virtual uint Step() const { return 1; }
	virtual uint Porty() const { return 8; }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE|C_PONT)); }

	bool CanAttaq(const ECBEntity* e)
	{
		if(e->Case() == Case() || e->IsCountryMaker() || e->Level() > L_GROUND ||
		   (!e->IsVehicule() && !e->IsInfantry() && !e->IsBuilding() && !e->IsNaval() &&
		   !e->IsPlane()))
			return false;
		else
			return true;
	}
	bool CanCreate(const ECBEntity*) { return false; }
	virtual bool WantDeploy() { return !(EventType() & ARM_ATTAQ); } ///< Default = false
	virtual bool WantAttaq(uint, uint, bool) { return (Deployed() ^ !!(EventType() & ARM_DEPLOY)); }
	bool IsVehicule() const { return true; }
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
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE)); }

	bool CanAttaq(const ECBEntity* e)
	{
		if((Parent() && Parent()->CanAttaq(e)) || (e->Level() == Level() && !e->IsHidden()) ||
		   (e->IsBuilding() && !e->IsCity() && !e->IsTerrain()) || e->Type() == E_BARBEDWIRE)
			return true;
		else
			return false;
	}
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsVehicule() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};


/********************************************************************************************
 *                               ECBJouano                                                  *
 ********************************************************************************************/
/** This is an enginer who can take enemey's buildings */
class ECBJouano : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBJouano) {}

	ENTITY_CONSTRUCTOR(ECBJouano) {}

/* Constantes */
public:

	virtual e_type Type() const { return E_JOUANO; }
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 1; }
	virtual uint Step() const { return 3; }
	virtual uint Porty() const { return 2; }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE|C_PONT)) && c->Entities()->Find(E_BARBEDWIRE, ARM_TYPE).empty(); }

	virtual bool CanInvest(const ECBEntity* e) const
	{
		if(e->Type() == E_MCDO && e->Deployed())
			return true;
		else
			return false;
	}

	bool CanAttaq(const ECBEntity* e) { return false; }
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsInfantry() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool WantDeploy() { return true; }
	virtual bool AddUnits(uint) { return false; }

	virtual bool CanBeCreated(ECBPlayer* pl) const;
};

/********************************************************************************************
 *                               ECBMcDo                                                    *
 ********************************************************************************************/
#define JOUANO_DESTROYTURN  3
/** This is a special Unit for USA nation */
class ECBMcDo : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBMcDo) : restDestroy(-1) {}

	ENTITY_CONSTRUCTOR(ECBMcDo), restDestroy(-1) {}

	enum data_t {
		DATA_INVESTED,
		DATA_EXOWNER,
		DATA_JOUANO
	};

/* Constantes */
public:

	virtual e_type Type() const { return E_MCDO; }
	virtual uint Cost() const { return 5000; }
	virtual uint InitNb() const { return 1000; }
	virtual uint Step() const { return Deployed() ? 0 : 4; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_USA); }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE|C_PONT)) && c->Entities()->Find(E_BARBEDWIRE, ARM_TYPE).empty(); }

	bool CanInvest(const ECBEntity* e) const
	{
		if(Deployed()) return false;
		if(e->Type() == E_CASERNE && !Like(e))
			return true;
		else
			return false;
	}
	bool CanAttaq(const ECBEntity* e) { return false; }
	virtual bool CanCreate(const ECBEntity*) { return false; }
	virtual void Create(ECBEntity*);
	virtual bool IsBuilding() const { return Deployed(); }
	bool IsInfantry() const { return !Deployed(); }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move, int) { return !Deployed(); }
	virtual bool IsCity() const { return Deployed(); }

/* Attributs */
public:

	int& RestDestroy() { return restDestroy; }

/* Variables privées */
protected:
	int restDestroy;
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
	virtual uint Step() const { return 6; }
	virtual uint Visibility() const { return 6; }
	virtual bool CanBeCreated(uint nation) const { return (nation == ECBPlayer::N_JAPON); }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE|C_PONT)) && c->Entities()->Find(E_BARBEDWIRE, ARM_TYPE).empty(); }

	bool CanInvest(const ECBEntity* e) const { return false; }
	bool CanAttaq(const ECBEntity* e) { return (e->IsInfantry()); }
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
	virtual uint Step() const { return 2; }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE|C_PONT)) && c->Entities()->Find(E_BARBEDWIRE, ARM_TYPE).empty(); }

	virtual bool CanInvest(const ECBEntity* e) const
	{
		if(e->IsBuilding() && !e->IsNaval() && !e->CanContain(this) && (!Like(e) || (e->Owner() == Owner() && e->Nb() < e->InitNb())) && !e->IsTerrain())
			return true;
		else
			return false;
	}

	bool CanAttaq(const ECBEntity* e) { return false; }
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
	virtual uint Cost() const { return 1000; }
	virtual uint InitNb() const { return 100; }
	virtual uint Step() const { return 2; }
	virtual bool CanWalkOn(ECBCase* c) const { return (c->Flags() & (C_TERRE|C_PONT)) && (c->Entities()->Find(E_BARBEDWIRE, ARM_TYPE).empty() == true); }

	bool CanAttaq(const ECBEntity* e)
	{
		if((Parent() && Parent()->CanAttaq(e)) || (e->Level() == Level() && !e->IsHidden() && e->Type() != E_MCDO) ||
		   (e->IsBuilding() && !e->IsCity() && !e->IsTerrain()))
			return true;
		else
			return false;
	}
	bool CanCreate(const ECBEntity*) { return false; }
	bool IsInfantry() const { return true; }
	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

#endif /* ECLIB_UNITS_H */
