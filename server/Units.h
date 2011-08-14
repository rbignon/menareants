/* server/Units.h - Header of Units.cpp
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

	virtual bool Contain(ECBEntity* entity);

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

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

/********************************************************************************************
 *                               ECBoat                                                     *
 ********************************************************************************************/
class ECBoat : public EContainer, public ECBBoat
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECBoat) {}

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

/********************************************************************************************
 *                               ECPlane                                                    *
 ********************************************************************************************/
class ECPlane : public EContainer, public ECBPlane
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECPlane) {}

	virtual bool WantDeploy();

	virtual bool WantUnContain(uint x, uint y, ECMove::Vector&);

	virtual int TurnMoney(ECBPlayer*);

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

/********************************************************************************************
 *                               ECBoeing                                                   *
 ********************************************************************************************/
class ECBoeing: public ECUnit, public ECBBoeing
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECBoeing) {}

	virtual bool WantDeploy();

	virtual bool WantMove(ECBMove::E_Move m, int i) { return Level() >= L_AIR ? ECUnit::WantMove(m,i) : false; }
	virtual bool WantAttaq(uint x, uint y, bool);
	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);
	std::vector<ECBEntity*> GetAttaquedEntities(ECBCase* c);
};

/********************************************************************************************
 *                               ECMissiLauncher                                            *
 ********************************************************************************************/
class ECMissiLauncher : public ECUnit, public ECBMissiLauncher
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECMissiLauncher) {}

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

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }

};

/********************************************************************************************
 *                               ECJouano                                                   *
 ********************************************************************************************/
class ECJouano : public ECUnit, public ECBJouano
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECJouano) {}

/* Methodes */
public:

	virtual bool WantDeploy();

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);
	virtual void Invest(ECBEntity* e);

	std::vector<ECBEntity*> GetAttaquedEntities(ECBCase* c);
};

/********************************************************************************************
 *                               ECMcDo                                                     *
 ********************************************************************************************/
class ECMcDo : public ECUnit, public ECBMcDo
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECMcDo), caserne(0), ex_owner(0) {}

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

	void Resynch(ECPlayer* pl);

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

	virtual bool WantAttaq(uint x, uint y, bool) { return false; }
};

#endif /* ECD_UNITS_H */
