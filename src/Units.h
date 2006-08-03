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

#ifndef EC_UNITS_H
#define EC_UNITS_H

#include "src/Map.h"
#include "lib/Units.h"
#include "Resources.h"
#include <map>

typedef ECBContainer   EContainer;

/********************************************************************************************
 *                                ECUnit                                                    *
 ********************************************************************************************/

class ECUnit : public ECEntity
{
/* Constructeur/Destructeur */
public:

	enum imgs_t {
		I_Up=ECMove::Up,
		I_Down=ECMove::Down,
		I_Left=ECMove::Left,
		I_Right=ECMove::Right,
		I_Attaq,
		I_Deployed,
		I_Reployed
	};
	typedef std::map<imgs_t, ECSpriteBase*> ImgList;

	ECUnit() : visual_step(0) {}

	ECUnit(uint vs) : visual_step(vs) {}

	virtual ~ECUnit();

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	void RefreshColor(Color last);

/* Mathodes protégées */
protected:
	bool MoveEffect(const std::vector<ECEntity*>&);

	void PutImage(imgs_t, ECSpriteBase*);

	ECSpriteBase* GetSprite(imgs_t t) { return images[t]; }

/* Variables protégées */
private:
	uint visual_step;
	ImgList images;
};

/********************************************************************************************
 *                                ECMissiLauncher                                           *
 ********************************************************************************************/
#define MISSILAUNCHER_VISUAL_STEP  1
#define MISSILAUNCHER_MISSILE_STEP 5
class ECMissiLauncher : public ECUnit, public ECBMissiLauncher
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECMissiLauncher)
		: ECUnit(MISSILAUNCHER_VISUAL_STEP), missile(this, MISSILAUNCHER_MISSILE_STEP)
	{}

	ENTITY_CONSTRUCTOR(ECMissiLauncher), ECUnit(MISSILAUNCHER_VISUAL_STEP), missile(this, MISSILAUNCHER_MISSILE_STEP)
	{
		PutImage(I_Up, Resources::MissiLauncher_Dos());
		PutImage(I_Down, Resources::MissiLauncher_Face());
		PutImage(I_Right, Resources::MissiLauncher_Right());
		PutImage(I_Left, Resources::MissiLauncher_Left());
		PutImage(I_Attaq, Resources::MissiLauncher_Right());
		PutImage(I_Deployed, Resources::MissiLauncher_Deployed());
		PutImage(I_Reployed, Resources::MissiLauncher_Reployed());
		missile.SetMissileUp(Resources::MissiLauncher_Missile_Up());
		missile.SetMissileDown(Resources::MissiLauncher_Missile_Down());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Lance-missile"; }
	virtual const char* Infos() const
		{ return "Véhicule lanceur de missiles à portée de 8 cases une fois déployé."; }
	virtual const char* Description() const
	{
		return "Le lance missile est une activité avec très peu de défense mais très efficace avec ses tirs à distance.\n"
		       "Il est très efficace contre les batiments, moyennement contre les vehicules et peu efficace contre "
		       "l'infanterie.";
	}
	virtual ECImage* Icon() const { return Resources::MissiLauncher_Icon(); }
	virtual bool WantAttaq(uint, uint, bool) { return Deployed(); }

	//virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

/* Methodes */
public:
	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

/* Variables privées */
private:
	ECMissile missile;
};

/********************************************************************************************
 *                                ECBoat                                                    *
 ********************************************************************************************/
#define BOAT_VISUAL_STEP  3
class ECBoat : public ECUnit, public ECBBoat
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBoat) {}

	ENTITY_CONSTRUCTOR(ECBoat), ECUnit(BOAT_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Boat_Dos());
		PutImage(I_Down, Resources::Boat_Face());
		PutImage(I_Right, Resources::Boat_Right());
		PutImage(I_Left, Resources::Boat_Left());
		PutImage(I_Attaq, Resources::Brouillard());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Bateau"; }
	virtual const char* Infos() const { return "Bateau de transport"; }
	virtual const char* Description() const
	{
		return "Ce bateau de transport peut contenir des unités d'infanterie.";
	}
	virtual ECImage* Icon() const { return Resources::Boat_Icon(); }
};


/********************************************************************************************
 *                                EChar                                                     *
 ********************************************************************************************/
#define CHAR_VISUAL_STEP  3
class EChar : public ECUnit, public ECBChar
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(EChar) {}

	ENTITY_CONSTRUCTOR(EChar), ECUnit(CHAR_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Char_Dos());
		PutImage(I_Down, Resources::Char_Face());
		PutImage(I_Right, Resources::Char_Right());
		PutImage(I_Left, Resources::Char_Left());
		PutImage(I_Attaq, Resources::Brouillard());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Char"; }
	virtual const char* Infos() const { return "Char d'assaut"; }
	virtual const char* Description() const
	{
		return "Le char est le vehicule de base et a la particularité d'être plus rapide qu'une armée, et coute moins cher "
		       "pour la même puissance. Cependant il ne peut traverser les ponts.";
	}
	virtual ECImage* Icon() const { return Resources::Char_Icon(); }
};

/********************************************************************************************
 *                                ECMcDo                                                    *
 ********************************************************************************************/
#define MCDO_VISUAL_STEP 2
class ECMcDo : public ECUnit, public ECBMcDo
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECMcDo) : invested(ECEntity::E_NONE) {}

	ENTITY_CONSTRUCTOR(ECMcDo), ECUnit(MCDO_VISUAL_STEP), invested(ECEntity::E_NONE)
	{
		PutImage(I_Up, Resources::McDo_Dos());
		PutImage(I_Down, Resources::McDo_Face());
		PutImage(I_Right, Resources::McDo_Right());
		PutImage(I_Left, Resources::McDo_Left());
		PutImage(I_Deployed, Resources::McDo_Caserne());
	}

/* Infos */
public:

	virtual const char* Name() const
	{
		return Deployed() ? "Caserne + McGerbale" : "Donald de McGerbale";
	}
	virtual const char* Infos() const
	{
		return Deployed() ? "" : "Emmenez-le sur une caserne pour qu'il y installe un McGerbale.";
	}
	virtual const char* Description() const
	{
		if(Deployed())
			return "Cette caserne est envahie par un McGerbale !!!";
		else
			return "Le Donald de McGerbale doit investir une caserne enemie pour y installer un McGerbale. Dès lors, "
			       "toute unité crée par cette caserne avancera deux fois moins vite, et les unités n'avançant initialement "
			       "que d'une case par tours seront (définitivement!) immobilisées, à cause de leur consommation de "
			       "nourriture abondante et lourde.\n"
			       "En outre, le propriétaire de la caserne infectée donnera chaque tours de l'argent à celui qui a posé "
			       "le McGerbale pour ce qui a été consommé.\n"
			       "Le seul moyen de virer le McGerbale est d'y envoyer un Jouano.";
	}
	virtual ECImage* Icon() const { return Deployed() ? Resources::Caserne_Icon() : Resources::McDo_Icon(); }

/* Methodes */
public:

	void RecvData(ECData data);
	std::string SpecialInfo();
	bool CanCreate(const ECBEntity* entity);

/* Variables privées */
public:
	ECEntity::e_type invested;
	std::string ex_owner;
};

/********************************************************************************************
 *                                ECTourist                                                 *
 ********************************************************************************************/
#define TOURIST_VISUAL_STEP 4
class ECTourist : public ECUnit, public ECBTourist
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECTourist) {}

	ENTITY_CONSTRUCTOR(ECTourist), ECUnit(TOURIST_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Tourist_Dos());
		PutImage(I_Down, Resources::Tourist_Face());
		PutImage(I_Right, Resources::Tourist_Right());
		PutImage(I_Left, Resources::Tourist_Left());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Touriste japonais"; }
	virtual const char* Infos() const { return "Il a un champs de vision élevé, et le terrain où il passe reste visible "
	                                           "en permanence."; }
	virtual const char* Description() const
	{
		return "Le touriste avance très vite et est munis d'un appareil photo spécial qui fait que partout où il passe "
		       "la zone restera pleinement visible même quand plus aucune unité ne sera présente sur place.";
	}
	virtual ECImage* Icon() const { return Resources::Tourist_Icon(); }

/* Methodes */
public:

	virtual void ChangeCase(ECBCase* new_case);
};

/********************************************************************************************
 *                                ECEnginer                                                 *
 ********************************************************************************************/
#define ENGINER_VISUAL_STEP 2
class ECEnginer : public ECUnit, public ECBEnginer
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECEnginer) {}

	ENTITY_CONSTRUCTOR(ECEnginer), ECUnit(ENGINER_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Enginer_Dos());
		PutImage(I_Down, Resources::Enginer_Face());
		PutImage(I_Right, Resources::Enginer_Right());
		PutImage(I_Left, Resources::Enginer_Left());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Ingénieur"; }
	virtual const char* Infos() const { return "Il peut capturer les batiments enemies ou réparer les votres."; }
	virtual const char* Description() const
	{
		return "Envoyez l'ingenieur sur un batiment enemie pour le capturer, ou sur un de vos batiment endomagé pour "
		       "le réparer.";
	}
	virtual ECImage* Icon() const { return Resources::Enginer_Icon(); }
};

/********************************************************************************************
 *                                ECArmy                                                    *
 ********************************************************************************************/
#define ARMY_VISUAL_STEP 2
class ECArmy : public ECUnit, public ECBArmy
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECArmy) {}

	ENTITY_CONSTRUCTOR(ECArmy), ECUnit(ARMY_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Army_Dos());
		PutImage(I_Down, Resources::Army_Face());
		PutImage(I_Right, Resources::Army_Right());
		PutImage(I_Left, Resources::Army_Left());
		PutImage(I_Attaq, Resources::Brouillard());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Armée"; }
	virtual const char* Infos() const { return "Armée de base"; }
	virtual const char* Description() const
	{
		return "C'est l'armée de base, que vous achetez par tranches de 100 hommes. Elle avance pas spécialement vite, mais "
		       "a l'avantage d'être très efficace dans les combats au corps à corps, et de pouvoir engager d'autres hommes "
		       "par groupes de 100 à faibles coût.";
	}
	virtual ECImage* Icon() const { return Resources::Army_Icon(); }
};

#endif /* EC_UNITS_H */
