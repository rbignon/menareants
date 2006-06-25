/* server/Units.h - Header of Units.cpp
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

#ifndef ECD_UNITS_H
#define ECD_UNITS_H

#include "server/Map.h"
#include "lib/Units.h"

/********************************************************************************************
 *                               ECUnit                                                     *
 ********************************************************************************************/
class ECUnit : public ECEntity
{
/* Constructeur/Destructeur */
public:

	ECUnit() : case_flags(0) {}

	ECUnit(unsigned f) : case_flags(f) {}

/* Methodes */
public:

	virtual bool WantMove(ECBMove::E_Move, bool force = false);

	virtual bool WantAttaq(uint x, uint y);

/* Variables protégées */
private:
	unsigned case_flags;
	
};

/********************************************************************************************
 *                               EContainer                                                 *
 ********************************************************************************************/
class EContainer : public ECBContainer, public ECUnit
{
/* Constructeur/Destructeur */
public:

	EContainer() {}

	EContainer(unsigned f) : ECUnit(f) {}

	virtual ~EContainer() {}

/* Methodes */
public:

	virtual void ChangeCase(ECBCase* new_case);

	virtual bool Attaq(std::vector<ECEntity*> entities);

	virtual bool WantContain(ECEntity*, ECMove::Vector&);

	virtual bool WantUnContain(uint x, uint y, ECMove::Vector&);

	virtual void Union(ECEntity*);

	void ReleaseShoot();
};

/********************************************************************************************
 *                               ECBoat                                                     *
 ********************************************************************************************/
class ECBoat : public EContainer, public ECBBoat
{
/* Constructeur/Destructeur */
public:

	BOAT_EMPTY_CONSTRUCTOR(ECBoat), EContainer(C_MER) {}

	BOAT_CONSTRUCTOR(ECBoat), EContainer(C_MER) {}

	ENTITY_CREATE_LAST(ECBoat);
};

/********************************************************************************************
 *                               ECMissiLauncher                                            *
 ********************************************************************************************/
class ECMissiLauncher : public ECUnit, public ECBMissiLauncher
{
/* Constructeur/Destructeur */
public:

	MISSILAUNCHER_EMPTY_CONSTRUCTOR(ECMissiLauncher), ECUnit(C_PONT|C_TERRE) {}

	MISSILAUNCHER_CONSTRUCTOR(ECMissiLauncher), ECUnit(C_PONT|C_TERRE) {}

	ENTITY_CREATE_LAST(ECMissiLauncher);

/* Methodes */
public:

	virtual bool Attaq(std::vector<ECEntity*> entities);

	virtual bool WantAttaq(uint x, uint y);

	virtual bool WantDeploy();
};

/********************************************************************************************
 *                               EChar                                                      *
 ********************************************************************************************/
class EChar : public ECUnit, public ECBChar
{
/* Constructeur/Destructeur */
public:

	CHAR_EMPTY_CONSTRUCTOR(EChar), ECUnit(C_TERRE) {}

	CHAR_CONSTRUCTOR(EChar), ECUnit(C_TERRE) {}

	ENTITY_CREATE_LAST(EChar);
};

/********************************************************************************************
 *                               ECArmy                                                     *
 ********************************************************************************************/
class ECArmy : public ECUnit, public ECBArmy
{
/* Constructeur/Destructeur */
public:

	ARMY_EMPTY_CONSTRUCTOR(ECArmy), ECUnit(C_TERRE|C_PONT) {}

	ARMY_CONSTRUCTOR(ECArmy), ECUnit(C_TERRE|C_PONT) {}

	ENTITY_CREATE_LAST(ECArmy);
};

#endif /* ECD_UNITS_H */
