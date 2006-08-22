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

class Color;

/********************************************************************************************
 *                                ECBatiment                                                *
 ********************************************************************************************/

class ECBatiment : public ECEntity
{
/* Constructeur/Destructeur */
public:

	ECBatiment() : img(0), explosion(0) {}

	ECBatiment(ECSpriteBase* b, ECSpriteBase* explosion = 0);
	virtual ~ECBatiment();

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) { return true; }

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual void RefreshColor(Color last);

private:
	ECSpriteBase *img;
	ECSpriteBase *explosion;
};

/********************************************************************************************
 *                                         ECTrees                                          *
 ********************************************************************************************/

class ECTrees : public ECBatiment, public ECBTrees
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECTrees) {}

	ENTITY_CONSTRUCTOR(ECTrees), ECBatiment(Resources::Trees_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Forêt"; }
	virtual const char* Infos() const { return "Quand une unité passe sous une forêt elle disparait aux yeux de l'enemie."; }
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

	ENTITY_CONSTRUCTOR(ECMine), ECBatiment(Resources::Mine_DesactFace(), Resources::Brouillard()), activeimg(0) {}

	~ECMine();

	virtual void Init();

/* Infos */
public:

	virtual const char* Name() const { return "Mine sous-terraine"; }
	virtual const char* Infos() const { return "La mine explose au passage d'une unité enemie."; }
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
 *                                ECNuclearSearch                                           *
 ********************************************************************************************/

class ECNuclearSearch : public ECBatiment, public ECBNuclearSearch
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECNuclearSearch) {}

	ENTITY_CONSTRUCTOR(ECNuclearSearch), ECBatiment(Resources::NuclearSearch_Face()) {}

	virtual void Init();

/* Infos */
public:

	virtual const char* Name() const { return "Centre de recherches nucléaire"; }
	virtual const char* Infos() const { return "Fabrication de missiles nucléaires"; }
	virtual const char* Description() const
	{
		return "Le centre fabrique des missiles nucléaires fréquements et peuvent être lancés depuis le Silo de lancement.";
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

	ENTITY_CONSTRUCTOR(ECSilo), ECBatiment(Resources::Silo_Face()), missile(this, SILO_MISSILE_STEP)
	{
		missile.SetMissileUp(Resources::Silo_Missile_Up());
		missile.SetMissileDown(Resources::Silo_Missile_Down());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Silo de lancement"; }
	virtual const char* Infos() const { return "Silo de lancement de missile nucléaire"; }
	virtual const char* Description() const
	{
		return "Le Silo de lancement peut lancer les missiles en stock dans le Centre de recherches nucléaire. Les dégats "
		       "causés par une bombe H sont d'une importance très grande (sur 6 cases avec pour centre la zone d'impact). "
		       "Vos unités ne sont pas à l'abris de vos propres tirs !!";
	}
	virtual ECImage* Icon() const { return Resources::Silo_Icon(); }

/* Méthodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual std::string SpecialInfo();

/* Variables privées */
private:
	ECMissile missile;
};

/********************************************************************************************
 *                                ECapitale                                                 *
 ********************************************************************************************/

class ECapitale : public ECBatiment, public ECBCapitale
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECapitale) {}

	ENTITY_CONSTRUCTOR(ECapitale), ECBatiment(Resources::Capitale_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Centre d'affaire"; }
	virtual const char* Infos() const { return "Rapporte deux fois plus d'argent que le centre ville"; }
	virtual const char* Description() const
	{
		return "Le centre d'affaire rapporte de l'argent chaques tours, deux fois plus que le centre ville. Celui qui "
		       "prend votre centre d'affaire devient possesseur de tous les quartiers de la ville";
	}
	virtual ECImage* Icon() const { return Resources::Capitale_Icon(); }
	virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNEDead(); }
};

/********************************************************************************************
 *                                ECity                                                     *
 ********************************************************************************************/

class ECity : public ECBatiment, public ECBCity
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECity) {}

	ENTITY_CONSTRUCTOR(ECity), ECBatiment(Resources::City_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Centre ville"; }
	virtual const char* Infos() const { return "Rapporte une certaine somme par tours"; }
	virtual const char* Description() const
	{
		return "Le centre ville rapporte de l'argent chaques tours. Celui qui prend votre centre ville devient possesseur de "
		       "tous les quartiers de la ville.";
	}
	virtual ECImage* Icon() const { return Resources::City_Icon(); }
	virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNEDead(); }
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

	ENTITY_CONSTRUCTOR(ECDefenseTower), ECBatiment(Resources::DefenseTower_Face()),
	                                    missile(this, DEFENSETOWER_MISSILE_STEP), miss(0), cible(0)
	{
		missile.SetMissileUp(Resources::MissiLauncher_Missile_Up());
		missile.SetMissileDown(Resources::MissiLauncher_Missile_Down());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Tour de défense"; }
	virtual const char* Infos() const { return "Peut tirer pour defendre la ville."; }
	virtual const char* Description() const
	{
		return "La tour de défense se trouve dès l'origine dans les villes. Elle fournit une défense non négligeable, meme si "
		       "elle a une porté plutot faible. Comptez sur les lances-missiles pour la détruire.";
	}
	virtual ECImage* Icon() const { return Resources::DefenseTower_Icon(); }
	virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNODead(); }

/* Methodes */
public:

	virtual void AfterDraw();
	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

/* Variables privées */
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

	ENTITY_CONSTRUCTOR(ECObelisk), ECBatiment(Resources::Obelisk_Face()), victim(0) {}

/* Infos */
public:

	virtual const char* Name() const { return "Obélisque du NOD"; }
	virtual const char* Infos() const { return "Bien plus puissantes que la tour de défense, les fans de C&C "
	                                           "reconnaitront."; }
	virtual const char* Description() const
	{
		return "L'Obélisque du NOD multiplie considérablement la puissance contre les vehicules et l'infanterie, et peut "
		       "maintenant toucher les batiments (hormis centre ville) !\n"
		       "Elle a une portée plus importante que la tour de défense, et sera beaucoup plus résistante aux attaques "
		       "contre elle.\n"
		       "L'obélisque du NOD est un clin d'oeil à un jeu culte.";
	}
	virtual ECImage* Icon() const { return Resources::Obelisk_Icon(); }
	virtual ECSpriteBase* DeadCase() const { return Resources::CaseCityNODead(); }

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

	ENTITY_CONSTRUCTOR(ECharFact), ECBatiment(Resources::CharFact_Face()) {}

	virtual void Init()
	{
		ECEntity::Init();
		Image()->SetAnim(true);
	}

/* Infos */
public:

	virtual const char* Name() const { return "Usine de chars"; }
	virtual const char* Infos() const { return "Construit des vehicules."; }
	virtual const char* Description() const
	{
		return "L'usine de chars peut construire les vehicules de base.";
	}
	virtual ECImage* Icon() const { return Resources::CharFact_Icon(); }
	virtual ECSpriteBase* DeadCase() const { return Resources::CaseCitySEDead(); }
};

/********************************************************************************************
 *                                ECaserne                                                  *
 ********************************************************************************************/

class ECaserne : public ECBatiment, public ECBCaserne
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECaserne) {}

	ENTITY_CONSTRUCTOR(ECaserne), ECBatiment(Resources::Caserne_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Caserne"; }
	virtual const char* Infos() const { return "Caserne pour entraîner des hommes"; }
	virtual const char* Description() const
	{
		return "La caserne construit l'infanterie de base.";
	}
	virtual ECImage* Icon() const { return Resources::Caserne_Icon(); }
	virtual ECSpriteBase* DeadCase() const { return Resources::CaseCitySODead(); }
};

/********************************************************************************************
 *                                ECShipyard                                                *
 ********************************************************************************************/

class ECShipyard : public ECBatiment, public ECBShipyard
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECShipyard) {}

	ENTITY_CONSTRUCTOR(ECShipyard), ECBatiment(Resources::Shipyard_Face()) {}

/* Infos */
public:

	virtual const char* Name() const { return "Chantier naval"; }
	virtual const char* Infos() const { return "Construit des bateaux"; }
	virtual const char* Description() const
	{
		return "Le chantier naval construit les unités navales.";
	}
	virtual ECImage* Icon() const { return Resources::Shipyard_Icon(); }
};


#endif /* EC_BATIMENTS_H */
