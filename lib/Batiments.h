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
 *                               ECBCharFact                                                *
 ********************************************************************************************/
/** This is a caserne */
#define CHARFACT_STEP                  0
#define CHARFACT_NB                    1000
#define CHARFACT_COST                  20000
#define CHARFACT_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CHARFACT, CHARFACT_COST)
#define CHARFACT_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CHARFACT_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_CHARFACT, CHARFACT_COST, CHARFACT_COST, _nb)
/** This is a simple army */
class ECBCharFact : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CHARFACT_EMPTY_CONSTRUCTOR(ECBCharFact) {}

	CHARFACT_CONSTRUCTOR(ECBCharFact) {}

	virtual ~ECBCharFact() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
			case E_CHAR:
				return true;
			default:
				return false;
		}
	}

	virtual const char* Qual() const { return "l'usine de chars"; }
	virtual uint InitNb() const { return CHARFACT_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move) { return false; }
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

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

/********************************************************************************************
 *                               ECBCaserne                                                 *
 ********************************************************************************************/
/** This is a caserne */
#define CASERNE_STEP                  0
#define CASERNE_NB                    500
#define CASERNE_COST                  9000
#define CASERNE_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CASERNE, CASERNE_COST)
#define CASERNE_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CASERNE_NB) \
                                       :  ECBEntity(_name, _owner, _case, E_CASERNE, CASERNE_COST, CASERNE_COST, _nb)
/** This is a simple army */
class ECBCaserne : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CASERNE_EMPTY_CONSTRUCTOR(ECBCaserne) {}

	CASERNE_CONSTRUCTOR(ECBCaserne) {}

	virtual ~ECBCaserne() {}

/* Constantes */
public:

	virtual bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
			case E_CHAR:
				return true;
			default:
				return false;
		}
	}

	virtual const char* Qual() const { return "la caserne"; }
	virtual uint InitNb() const { return CASERNE_NB; }
	virtual bool IsBuilding() const { return true; }
	virtual bool AddUnits(uint) { return false; }
	virtual bool WantMove(ECBMove::E_Move) { return false; }
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

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

#endif /* ECLIB_BATIMENTS_H */
