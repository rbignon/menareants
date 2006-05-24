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

/********************************************************************************************
 *                               ECBMissiLauncher                                           *
 ********************************************************************************************/
#define MISSILAUNCHER_STEP                  1
#define MISSILAUNCHER_NB                    100
#define MISSILAUNCHER_COST                  6000
#define MISSILAUNCHER_PORTY                 8
#define MISSILAUNCHER_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_MISSILAUNCHER, MISSILAUNCHER_COST)
#define MISSILAUNCHER_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, \
                                                uint _nb = MISSILAUNCHER_NB) \
                                            :  ECBEntity(_name, _owner, _case, E_MISSILAUNCHER, MISSILAUNCHER_STEP, \
                                                       MISSILAUNCHER_COST, _nb)
/** This is a simple army */
class ECBMissiLauncher : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	MISSILAUNCHER_EMPTY_CONSTRUCTOR(ECBMissiLauncher) {}

	MISSILAUNCHER_CONSTRUCTOR(ECBMissiLauncher) {}

	virtual ~ECBMissiLauncher() {}

/* Constantes */
public:

	bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
			case E_CHAR:
			case E_CASERNE:
			case E_CHARFACT:
			case E_MISSILAUNCHER:
				return true;
			default:
				return false;
		}
	}
	virtual const char* Qual() const { return "le lance-missiles"; }
	bool CanCreate(const ECBEntity*) { return false; }
	uint InitNb() const { return MISSILAUNCHER_NB; }
	virtual bool WantDeploy() { return !(EventType() & ARM_ATTAQ); } ///< Default = false
	virtual bool WantAttaq(uint, uint) { return Deployed(); }

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
#define CHAR_STEP                  3
#define CHAR_NB                    1500
#define CHAR_COST                  20000
#define CHAR_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_CHAR, CHAR_COST)
#define CHAR_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = CHAR_NB) \
                                     :  ECBEntity(_name, _owner, _case, E_CHAR, CHAR_STEP, CHAR_COST, _nb)
/** This is a simple army */
class ECBChar : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	CHAR_EMPTY_CONSTRUCTOR(ECBChar) {}

	CHAR_CONSTRUCTOR(ECBChar) {}

	virtual ~ECBChar() {}

/* Constantes */
public:

	bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
			case E_CHAR:
			case E_CASERNE:
			case E_MISSILAUNCHER:
			case E_CHARFACT:
				return true;
			default:
				return false;
		}
	}
	virtual const char* Qual() const { return "le char"; }
	bool CanCreate(const ECBEntity*) { return false; }
	uint InitNb() const { return CHAR_NB; }

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
#define ARMY_STEP                  2
#define ARMY_NB                    100
#define ARMY_COST                  2000
#define ARMY_EMPTY_CONSTRUCTOR(x)  x() : ECBEntity(E_ARMY, ARMY_COST)
#define ARMY_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb = ARMY_NB) \
                                     :  ECBEntity(_name, _owner, _case, E_ARMY, ARMY_STEP, ARMY_COST, _nb)
/** This is a simple army */
class ECBArmy : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ARMY_EMPTY_CONSTRUCTOR(ECBArmy) {}

	ARMY_CONSTRUCTOR(ECBArmy) {}

	virtual ~ECBArmy() {}

/* Constantes */
public:

	bool CanAttaq(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case E_ARMY:
			case E_CASERNE:
			case E_CHARFACT:
			case E_CHAR:
			case E_MISSILAUNCHER:
				return true;
			default:
				return false;
		}
	}
	virtual const char* Qual() const { return "l'armée"; }
	bool CanCreate(const ECBEntity*) { return false; }
	uint InitNb() const { return ARMY_NB; }

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:
};

#endif /* ECLIB_UNITS_H */
