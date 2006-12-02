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
/* Methodes */
public:

	virtual bool WantMove(ECBMove::E_Move, int flags = 0);

	virtual bool WantAttaq(uint x, uint y, bool force = false);
};

/********************************************************************************************
 *                               EContainer                                                 *
 ********************************************************************************************/
class EContainer : public virtual ECBContainer, public ECUnit
{
/* Constructeur/Destructeur */
public:

	virtual ~EContainer() {}

/* Methodes */
public:

	virtual void ChangeCase(ECBCase* new_case);

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);

	virtual bool WantContain(ECEntity*, ECMove::Vector&);

	virtual bool WantUnContain(uint x, uint y, ECMove::Vector&);

	virtual void Union(ECEntity*);

	void ReleaseShoot();
};

/********************************************************************************************
 *                               ECTrain                                                    *
 ********************************************************************************************/
class ECTrain : public EContainer, public ECBTrain
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECTrain) {}

	ENTITY_CREATE_LAST(ECTrain);
};

/********************************************************************************************
 *                               ECBoat                                                     *
 ********************************************************************************************/
class ECBoat : public EContainer, public ECBBoat
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECBoat) {}

	ENTITY_CREATE_LAST(ECBoat);
};

/********************************************************************************************
 *                               ECPlane                                                    *
 ********************************************************************************************/
class ECPlane : public EContainer, public ECBPlane
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECPlane) {}

	ENTITY_CREATE_LAST(ECPlane);

	virtual bool WantDeploy();

	virtual bool WantUnContain(uint x, uint y, ECMove::Vector&);
};

/********************************************************************************************
 *                               ECMissiLauncher                                            *
 ********************************************************************************************/
class ECMissiLauncher : public ECUnit, public ECBMissiLauncher
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECMissiLauncher) {}

	ENTITY_CREATE_LAST(ECMissiLauncher);

/* Methodes */
public:

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);

	virtual bool WantAttaq(uint x, uint y, bool);

	virtual bool WantDeploy();
};

/********************************************************************************************
 *                               EChar                                                      *
 ********************************************************************************************/
class EChar : public ECUnit, public ECBChar
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(EChar) {}

	ENTITY_CREATE_LAST(EChar);
};

/********************************************************************************************
 *                               ECJouano                                                   *
 ********************************************************************************************/
class ECJouano : public ECUnit, public ECBJouano
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECJouano) {}

	ENTITY_CREATE_LAST(ECJouano);

/* Methodes */
public:

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event) { return false; }
	virtual void Invest(ECBEntity* e);
};

/********************************************************************************************
 *                               ECMcDo                                                     *
 ********************************************************************************************/
class ECMcDo : public ECUnit, public ECBMcDo
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECMcDo), caserne(0), ex_owner(0) {}

	ENTITY_CREATE_LAST(ECMcDo);

/* Methodes */
public:

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event) { return false; }
	virtual bool WantMove(ECBMove::E_Move m, int i) { return !Deployed() ? ECUnit::WantMove(m,i) : false; }
	virtual void Invest(ECBEntity* e);
	virtual bool CanCreate(const ECBEntity*);
	virtual int TurnMoney(ECBPlayer*);
	virtual void ChangeOwner(ECBPlayer*);
	virtual void Played();

/* Attributs */
public:

	ECPlayer* ExOwner() const { return ex_owner; }

/* Variables privées */
private:
	ECEntity* caserne;
	ECPlayer* ex_owner;
};

/********************************************************************************************
 *                               ECEnginer                                                  *
 ********************************************************************************************/
class ECTourist : public ECUnit, public ECBTourist
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECTourist) {}

	ENTITY_CREATE_LAST(ECTourist);

/* Methodes */
public:

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

/********************************************************************************************
 *                               ECEnginer                                                  *
 ********************************************************************************************/
class ECEnginer : public ECUnit, public ECBEnginer
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECEnginer) {}

	ENTITY_CREATE_LAST(ECEnginer);

/* Methodes */
public:

	/** Invest a building */
	virtual void Invest(ECBEntity* e);

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

/********************************************************************************************
 *                               ECArmy                                                     *
 ********************************************************************************************/
class ECArmy : public ECUnit, public ECBArmy
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECArmy) {}

	ENTITY_CREATE_LAST(ECArmy);
};

#endif /* ECD_UNITS_H */
