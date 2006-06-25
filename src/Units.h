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

	void RefreshColor(SDL_Color last);

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

	MISSILAUNCHER_EMPTY_CONSTRUCTOR(ECMissiLauncher), ECUnit(MISSILAUNCHER_VISUAL_STEP), missile(0) {}

	MISSILAUNCHER_CONSTRUCTOR(ECMissiLauncher), ECUnit(MISSILAUNCHER_VISUAL_STEP), missile(0)
	{
		PutImage(I_Up, Resources::MissiLauncher_Dos());
		PutImage(I_Down, Resources::MissiLauncher_Face());
		PutImage(I_Right, Resources::MissiLauncher_Right());
		PutImage(I_Left, Resources::MissiLauncher_Left());
		PutImage(I_Attaq, Resources::MissiLauncher_Right());
		PutImage(I_Deployed, Resources::MissiLauncher_Deployed());
		PutImage(I_Reployed, Resources::MissiLauncher_Reployed());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Lance-missile"; }
	virtual const char* Infos() const
		{ return "Véhicule lanceur de missiles à portée de 8 cases une fois déployé."; }
	virtual ECImage* Icon() const { return Resources::MissiLauncher_Icon(); }
	virtual bool WantAttaq(uint, uint) { return Deployed() && !IWantDeploy(); }

	//virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

/* Methodes */
public:
	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual void Draw();

/* Variables privées */
private:
	ECSprite* missile;

	void SetMissile(ECSpriteBase* c);
};

/********************************************************************************************
 *                                ECBoat                                                    *
 ********************************************************************************************/
#define BOAT_VISUAL_STEP  3
class ECBoat : public ECUnit, public ECBBoat
{
/* Constructeur/Destructeur */
public:

	BOAT_EMPTY_CONSTRUCTOR(ECBoat), ECUnit(BOAT_VISUAL_STEP) {}

	BOAT_CONSTRUCTOR(ECBoat), ECUnit(BOAT_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Boat_Dos());
		PutImage(I_Down, Resources::Boat_Face());
		PutImage(I_Right, Resources::Boat_Right());
		PutImage(I_Left, Resources::Boat_Left());
		PutImage(I_Attaq, Resources::Boat_Face());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Bateau"; }
	virtual const char* Infos() const { return "Bateau de transport"; }
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

	CHAR_EMPTY_CONSTRUCTOR(EChar), ECUnit(CHAR_VISUAL_STEP) {}

	CHAR_CONSTRUCTOR(EChar), ECUnit(CHAR_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Char_Dos());
		PutImage(I_Down, Resources::Char_Face());
		PutImage(I_Right, Resources::Char_Right());
		PutImage(I_Left, Resources::Char_Left());
		PutImage(I_Attaq, Resources::Char_Face());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Char"; }
	virtual const char* Infos() const { return "Char d'assaut"; }
	virtual ECImage* Icon() const { return Resources::Char_Icon(); }
};

/********************************************************************************************
 *                                ECArmy                                                    *
 ********************************************************************************************/
#define ARMY_VISUAL_STEP 2
class ECArmy : public ECUnit, public ECBArmy
{
/* Constructeur/Destructeur */
public:

  ARMY_EMPTY_CONSTRUCTOR(ECArmy), ECUnit(ARMY_VISUAL_STEP) {}

	ARMY_CONSTRUCTOR(ECArmy), ECUnit(ARMY_VISUAL_STEP)
	{
		PutImage(I_Up, Resources::Army_Dos());
		PutImage(I_Down, Resources::Army_Face());
		PutImage(I_Right, Resources::Army_Right());
		PutImage(I_Left, Resources::Army_Left());
		PutImage(I_Attaq, Resources::Army_Face());
	}

/* Infos */
public:

	virtual const char* Name() const { return "Armée"; }
	virtual const char* Infos() const { return "Armée de base"; }
	virtual ECImage* Icon() const { return Resources::Army_Icon(); }
};

#endif /* EC_UNITS_H */
