/* server/Batiments.h - Header of Batiments.cpp
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

#ifndef ECD_BATIMENTS_H
#define ECD_BATIMENTS_H

#include "server/Map.h"
#include "lib/Batiments.h"

/********************************************************************************************
 *                                        ECBarbedWire                                      *
 ********************************************************************************************/
class ECBarbedWire : public ECEntity, public ECBBarbedWire
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECBarbedWire) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                                        ECRail                                            *
 ********************************************************************************************/
class ECRail : public ECEntity, public ECBRail
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECRail) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                                        ECTrees                                           *
 ********************************************************************************************/
class ECTrees : public ECEntity, public ECBTrees
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECTrees) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                                        ECMine                                            *
 ********************************************************************************************/
class ECMine : public ECEntity, public ECBMine
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECMine) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }

	virtual void Played();

	void Resynch(ECPlayer* pl);

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);
};

/********************************************************************************************
 *                               ECEiffelTower                                              *
 ********************************************************************************************/
class ECEiffelTower : public ECEntity, public ECBEiffelTower
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECEiffelTower) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};


/********************************************************************************************
 *                               ECRadar                                                    *
 ********************************************************************************************/
class ECRadar : public ECEntity, public ECBRadar
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECRadar) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECNuclearSearch                                            *
 ********************************************************************************************/
class ECNuclearSearch : public ECEntity, public ECBNuclearSearch
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECNuclearSearch) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }

	virtual void Played();

	void Resynch(ECPlayer* pl);

	void RemoveOneMissile();
};

/********************************************************************************************
 *                                 ECSilo                                                   *
 ********************************************************************************************/
class ECSilo : public ECEntity, public ECBSilo
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECSilo) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);

	virtual void Played();

	virtual bool WantAttaq(uint x, uint y, bool);

	void Resynch(ECPlayer* pl);

/* Attributs */
public:

	std::vector<ECBEntity*> GetAttaquedEntities(ECBCase* c) const;
};

/********************************************************************************************
 *                                 ECGulag                                                  *
 ********************************************************************************************/
class ECGulag: public ECEntity, public ECBGulag
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECGulag) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }

	virtual void Played();

	void Resynch(ECPlayer* pl);

/* Attributes */
public:
	void AddPrisoners(uint nb) { nb_prisoners += nb; }
};

/********************************************************************************************
 *                                 ECCavern                                                  *
 ********************************************************************************************/
class ECavern: public EContainer, public ECBCavern
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECavern) {}

/* Methodes */
public:

	virtual bool Contain(ECBEntity* e);
	void ReleaseShoot();

};

/********************************************************************************************
 *                               ECMegalopole                                               *
 ********************************************************************************************/
class ECMegalopole : public ECEntity, public ECBMegalopole
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECMegalopole) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECapitale                                                  *
 ********************************************************************************************/
class ECapitale : public ECEntity, public ECBCapitale
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECapitale) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECity                                                      *
 ********************************************************************************************/
class ECity : public ECEntity, public ECBCity
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECity) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECDefenseTower                                             *
 ********************************************************************************************/
class ECDefenseTower : public ECEntity, public ECBDefenseTower
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECDefenseTower) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);

	virtual bool WantAttaq(uint x, uint y, bool);
};

/********************************************************************************************
 *                                    ECObelisk                                             *
 ********************************************************************************************/
class ECObelisk : public ECEntity, public ECBObelisk
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECObelisk) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }

	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);

	virtual bool WantAttaq(uint x, uint y, bool);
};


/********************************************************************************************
 *                               ECharFact                                                  *
 ********************************************************************************************/
class ECharFact : public ECEntity, public ECBCharFact
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECharFact) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECaserne                                                   *
 ********************************************************************************************/
class ECaserne : public ECEntity, public ECBCaserne
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECaserne) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};

/********************************************************************************************
 *                               ECAirPort                                                  *
 ********************************************************************************************/
class ECAirPort : public ECEntity, public ECBAirPort
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECAirPort) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};


/********************************************************************************************
 *                               ECShipyard                                                 *
 ********************************************************************************************/
class ECShipyard : public ECEntity, public ECBShipyard
{
/* Constructeur/Destructeur */
public:

	ENTITY_CONSTRUCTOR(ECShipyard) {}

/* Methodes */
public:

	virtual void Union(ECEntity*) { return; }
};


#endif /* ECD_BATIMENTS_H */
