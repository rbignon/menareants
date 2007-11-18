/* src/Units.h - Header of Units.cpp
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

#ifndef EC_BATIMENTS_H
#define EC_BATIMENTS_H

#include "src/Map.h"
#include "lib/Batiments.h"
#include "Resources.h"
#include "i18n.h"
#include <assert.h>

class Color;

/********************************************************************************************
 *                                ECBatiment                                                *
 ********************************************************************************************/

class ECBatiment : public ECEntity
{
/* Constructeur/Destructeur */
public:

	ECBatiment() : explosion(0) {}

	ECBatiment(ECSpriteBase* explosion);

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

private:
	ECSpriteBase *explosion;
};

/********************************************************************************************
 *                                         ECBarbedWire                                     *
 ********************************************************************************************/

class ECBarbedWire : public ECBatiment, public ECBBarbedWire
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBarbedWire) {}

	ENTITY_CONSTRUCTOR(ECBarbedWire) {}

	void UpdateImages() { PutImage(I_Down, (Resources::BarbedWire_Horiz())); }

	virtual void Created();

/* Methodes */
public:

	void Played();
	void FindMyImage(bool make_others = true);

/* Infos */
public:

	virtual const char* Name() const { return _("Barbed Wire"); }
	virtual const char* Infos() const { return _("You can put a barbed wire to prevent the infantry from walking."); }
	virtual const char* Qual() const { return _("barbed wire"); }
	virtual const char* Description() const { return Infos(); }
	virtual ECImage* Icon() const { return Resources::BarbedWire_Icon(); }
	virtual bool CanBeSelected() const { return false; }
};

/********************************************************************************************
 *                                         ECRail                                           *
 ********************************************************************************************/

class ECRail : public ECBatiment, public ECBRail
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECRail) {}

	ENTITY_CONSTRUCTOR(ECRail) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Rail_Horiz())); }

	virtual void Created();

/* Methodes */
public:

	void Played();
	void FindMyImage(bool make_others = true);

/* Infos */
public:

	virtual const char* Name() const { return _("Rail"); }
	virtual const char* Infos() const { return _("Trains are only able to move on rails."); }
	virtual const char* Qual() const { return _("rail"); }
	virtual const char* Description() const { return Infos(); }
	virtual ECImage* Icon() const { return Resources::Rail_Icon(); }
	virtual bool CanBeSelected() const { return false; }
};

/********************************************************************************************
 *                                         ECTrees                                          *
 ********************************************************************************************/

class ECTrees : public ECBatiment, public ECBTrees
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECTrees) {}

	ENTITY_CONSTRUCTOR(ECTrees) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Trees_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("Forest"); }
	virtual const char* Qual() const { return _("forest"); }
	virtual const char* Infos() const { return _("Under a forest, infantry units become invisible for enemies."); }
	virtual const char* Description() const { return Infos(); }
	virtual ECImage* Icon() const { return Resources::Trees_Icon(); }
	virtual bool CanBeSelected() const { return false; }
	virtual bool OnTop() const { return true; }
	virtual bool IsHiddenByMe(ECBEntity* e) const
	{
		assert(e);
		if(e->IsInfantry()) return true;
		else return false;
	}
};

/********************************************************************************************
 *                                         ECMine                                           *
 ********************************************************************************************/

class ECMine : public ECBatiment, public ECBMine
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECMine) : activeimg(0) {}

	ENTITY_CONSTRUCTOR(ECMine), ECBatiment(Resources::Brouillard()), activeimg(0) {}

	void UpdateImages() { PutImage(I_Down, Resources::Mine_DesactFace()); }

	~ECMine();

	virtual void Init();

/* Infos */
public:

	virtual const char* Name() const { return _("Underground mine"); }
	virtual const char* Qual() const { return _("mine"); }
	virtual const char* Infos() const { return _("The mine explodes when an unit passes on it."); }
	virtual const char* Description() const { return Infos(); }
	virtual ECImage* Icon() const { return Resources::Mine_Icon(); }

/* Methodes */
public:

	virtual void RecvData(ECData);

	virtual std::string SpecialInfo();

private:
	ECSpriteBase* activeimg;
};

/********************************************************************************************
 *                                ECEiffelTower                                             *
 ********************************************************************************************/

class ECEiffelTower : public ECBatiment, public ECBEiffelTower
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECEiffelTower) {}

	ENTITY_CONSTRUCTOR(ECEiffelTower) {}

	void UpdateImages() { PutImage(I_Down, (Resources::EiffelTower_Face())); }

	virtual void Init();

	~ECEiffelTower();

/* Infos */
public:

	virtual const char* Name() const { return _("Eiffel Tower"); }
	virtual const char* Qual() const { return _("eiffel tower"); }
	virtual const char* Infos() const { return _("Show all units on map."); }
	virtual const char* Description() const
	{
		return _("Eiffel Tower shows all units on the map.");
	}
	virtual ECImage* Icon() const { return Resources::EiffelTower_Icon(); }
};

/********************************************************************************************
 *                                ECRadar                                                   *
 ********************************************************************************************/

class ECRadar : public ECBatiment, public ECBRadar
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECRadar) {}

	ENTITY_CONSTRUCTOR(ECRadar) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Radar_Face())); }

	virtual void Init();

	~ECRadar();

/* Infos */
public:

	virtual const char* Name() const { return _("Radar"); }
	virtual const char* Qual() const { return _("radar"); }
	virtual const char* Infos() const { return _("Show all map."); }
	virtual const char* Description() const
	{
		return _("This radar shows all the map. If you want to see all units on the map, upgrade it.");
	}
	virtual ECImage* Icon() const { return Resources::Radar_Icon(); }
};

/********************************************************************************************
 *                                ECNuclearSearch                                           *
 ********************************************************************************************/

class ECNuclearSearch : public ECBatiment, public ECBNuclearSearch
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECNuclearSearch) {}

	ENTITY_CONSTRUCTOR(ECNuclearSearch) {}

	void UpdateImages() { PutImage(I_Down, (Resources::NuclearSearch_Face())); }

	virtual void Init();

/* Infos */
public:

	virtual const char* Name() const { return _("Nuclear research center"); }
	virtual const char* Qual() const { return _("nuclear research center"); }
	virtual const char* Infos() const { return _("Nuclear missile manufacturing."); }
	virtual const char* Description() const
	{
		return _("The center regularly builds a missile which can be launched from the nuclear silo.");
	}
	virtual ECImage* Icon() const { return Resources::NuclearSearch_Icon(); }

/* Methodes */
public:

	virtual void RecvData(ECData);

	virtual std::string SpecialInfo();
};

/********************************************************************************************
 *                                         ECSilo                                           *
 ********************************************************************************************/
#define SILO_MISSILE_STEP 3
class ECSilo : public ECBatiment, public ECBSilo
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECSilo) : missile(this, SILO_MISSILE_STEP) {}

	ENTITY_CONSTRUCTOR(ECSilo), ECBatiment(), missile(this, SILO_MISSILE_STEP)
	{
		missile.SetMissileUp(Resources::Silo_Missile_Up());
		missile.SetMissileDown(Resources::Silo_Missile_Down());
	}

	void UpdateImages()
	{
		PutImage(I_Down, Resources::Silo_Face());
	}

	void Init()
	{
		ECEntity::Init();
		ECBSilo::Init();
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Launching silo"); }
	virtual const char* Qual() const { return _("launching silo"); }
	virtual const char* Infos() const { return _("Nuclear missile launching silo"); }
	virtual const char* Description() const
	{
		return _("The launching silo can launch the nuclear missile avalaible in the nuclear research center stock. The damages caused by a H bomb are very important (6 squares around impact). Your units are not immunized.");
	}
	virtual ECImage* Icon() const { return Resources::Silo_Icon(); }

/* M�hodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual std::string SpecialInfo();

	virtual void RecvData(ECData);

/* Variables priv�s */
private:
	ECMissile missile;
};

/********************************************************************************************
 *                                ECMegalopole                                              *
 ********************************************************************************************/

class ECMegalopole : public ECBatiment, public ECBMegalopole
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECMegalopole) {}

	ENTITY_CONSTRUCTOR(ECMegalopole) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Megalopole_Face())); }

	virtual void Init()
	{
		ECEntity::Init();
		Image()->SetAnim(true);
	}


/* Infos */
public:

	virtual const char* Name() const { return _("Megalopolis"); }
	virtual const char* Qual() const { return _("megalopolis"); }
	virtual const char* Infos() const { return _("Five times more income than the city center."); }
	virtual const char* Description() const
	{
		return _("The megalopolis earns you money each turn, FIVE times more than the city center, and 2,5 times more than the business center. Whoever takes your megalopolis becomes the owner of all the districts of the city.");
	}
	virtual ECImage* Icon() const { return Resources::Megalopole_Icon(); }
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNEDead(); }
	virtual std::string SpecialInfo();
};

/********************************************************************************************
 *                                ECapitale                                                 *
 ********************************************************************************************/

class ECapitale : public ECBatiment, public ECBCapitale
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECapitale) {}

	ENTITY_CONSTRUCTOR(ECapitale) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Capitale_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("Business center"); }
	virtual const char* Qual() const { return _("business center"); }
	virtual const char* Infos() const { return _("Two times more income than the city center."); }
	virtual const char* Description() const
	{
		return _("The business center earns you money each turn, two times more than the city center. Whoever takes your business center becomes the owner of all the districts of the city.");
	}
	virtual ECImage* Icon() const { return Resources::Capitale_Icon(); }
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNEDead(); }
	virtual std::string SpecialInfo();
};

/********************************************************************************************
 *                                ECity                                                     *
 ********************************************************************************************/

class ECity : public ECBatiment, public ECBCity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECity) {}

	ENTITY_CONSTRUCTOR(ECity) {}

	void UpdateImages() { PutImage(I_Down, (Resources::City_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("City center"); }
	virtual const char* Qual() const { return _("city center"); }
	virtual const char* Infos() const { return _("Earns you some money each turn"); }
	virtual const char* Description() const
	{
		return _("The city center earns you money each turn. Whoever takes your business center becomes the owner of all the districts of the city.");
	}
	virtual ECImage* Icon() const { return Resources::City_Icon(); }
	virtual std::string SpecialInfo();
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNEDead(); }
};

/********************************************************************************************
 *                                ECDefenseTower                                            *
 ********************************************************************************************/
#define DEFENSETOWER_MISSILE_STEP 4
class ECDefenseTower : public ECBatiment, public ECBDefenseTower
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECDefenseTower) : missile(this, DEFENSETOWER_MISSILE_STEP), miss(0), cible(0) {}

	ENTITY_CONSTRUCTOR(ECDefenseTower), ECBatiment(),
	                                    missile(this, DEFENSETOWER_MISSILE_STEP), miss(0), cible(0)
	{
		missile.SetMissileUp(Resources::MissiLauncher_Missile_Up());
		missile.SetMissileDown(Resources::MissiLauncher_Missile_Down());
	}

	void UpdateImages()
	{
		PutImage(I_Down, Resources::DefenseTower_Face());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Defense tower"); }
	virtual const char* Qual() const { return _("defense tower"); }
	virtual const char* Infos() const { return _("Can fire to defend your city."); }
	virtual const char* Description() const
	{
		return _("The defense tower is originally located in cities. It provides a non-negligible defense, even if it has a short reach. Use missile launchers to destroy it.");
	}
	virtual ECImage* Icon() const { return Resources::DefenseTower_Icon(); }
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNODead(); }

/* Methodes */
public:

	virtual void AfterDraw();
	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

/* Variables priv�s */
private:
	ECMissile missile;
	unsigned int miss;
	ECase* cible;
};

/********************************************************************************************
 *                                ECDefenseTower                                            *
 ********************************************************************************************/
class ECObelisk : public ECBatiment, public ECBObelisk
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECObelisk) : victim(0) {}

	ENTITY_CONSTRUCTOR(ECObelisk), victim(0) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Obelisk_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("NOD's obelisk"); }
	virtual const char* Qual() const { return _("obelisk"); }
	virtual const char* Infos() const { return _("Way more powerful than defense towers, C&C fans will recognize it."); }
	virtual const char* Description() const
	{
		return _("NOD's obelisk considerably multiplies your attack power against vehicles and infantry, and can now attack buildings (except the city centers)!\nIt has a better reach than the defense towers, and is more resistant to attacks.\nThe NOD's obelisk is a reference to a cult game.");
	}
	virtual ECImage* Icon() const { return Resources::Obelisk_Icon(); }
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNODead(); }

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual void AfterDraw();

private:
	ECImage img;
	ECase* victim;
};

/********************************************************************************************
 *                                ECharFact                                                 *
 ********************************************************************************************/

class ECharFact : public ECBatiment, public ECBCharFact
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECharFact) {}

	ENTITY_CONSTRUCTOR(ECharFact) {}

	void UpdateImages() { PutImage(I_Down, (Resources::CharFact_Face())); }

	virtual void Init()
	{
		ECEntity::Init();
		Image()->SetAnim(true);
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Tank factory"); }
	virtual const char* Qual() const { return _("tank factory"); }
	virtual const char* Infos() const { return _("Builds vehicles."); }
	virtual const char* Description() const
	{
		return _("The tank factory can build basic vehicles.");
	}
	virtual ECImage* Icon() const { return Resources::CharFact_Icon(); }
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCitySEDead(); }
};

/********************************************************************************************
 *                                ECaserne                                                  *
 ********************************************************************************************/

class ECaserne : public ECBatiment, public ECBCaserne
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECaserne) {}

	ENTITY_CONSTRUCTOR(ECaserne) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Caserne_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("Barracks"); }
	virtual const char* Qual() const { return _("barracks"); }
	virtual const char* Infos() const { return _("Used to train men."); }
	virtual const char* Description() const
	{
		return _("The barracks builds basic infantry.");
	}
	virtual ECImage* Icon() const { return Resources::Caserne_Icon(); }
	//virtual ECSpriteBase* DeadCase() const { return Resources::CaseCitySODead(); }
};

/********************************************************************************************
 *                                ECAirPort                                                 *
 ********************************************************************************************/

class ECAirPort : public ECBatiment, public ECBAirPort
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECAirPort) {}

	ENTITY_CONSTRUCTOR(ECAirPort) {}

	void UpdateImages() { PutImage(I_Down, (Resources::AirPort_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("AirPort"); }
	virtual const char* Qual() const { return _("airport"); }
	virtual const char* Infos() const { return _("Can build planes."); }
	virtual const char* Description() const
	{
		return _("The airport can be used to create planes.");
	}
	virtual ECImage* Icon() const { return Resources::AirPort_Icon(); }
};

/********************************************************************************************
 *                                ECShipyard                                                *
 ********************************************************************************************/

class ECShipyard : public ECBatiment, public ECBShipyard
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECShipyard) {}

	ENTITY_CONSTRUCTOR(ECShipyard) {}

	void UpdateImages() { PutImage(I_Down, (Resources::Shipyard_Face())); }

/* Infos */
public:

	virtual const char* Name() const { return _("Shipyard"); }
	virtual const char* Qual() const { return _("shipyard"); }
	virtual const char* Infos() const { return _("Builds ships."); }
	virtual const char* Description() const
	{
		return _("The shipyard is able to build ships.");
	}
	virtual ECImage* Icon() const { return Resources::Shipyard_Icon(); }
};

#endif /* EC_BATIMENTS_H */
