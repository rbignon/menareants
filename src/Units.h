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
		I_Attaq
	};
	typedef std::map<imgs_t, ECSpriteBase*> ImgList;

	ECUnit() : visual_step(0) {}

	ECUnit(uint vs) : visual_step(vs) {}

	virtual ~ECUnit();

/* Methodes */
public:

	void PutImage(imgs_t, ECSpriteBase*);

	virtual bool BeforeEvent(const std::vector<ECEntity*>&);

	virtual bool MakeEvent(const std::vector<ECEntity*>&);

	virtual bool AfterEvent(const std::vector<ECEntity*>&);

/* Variables protégées */
private:
	uint visual_step;
	ImgList images;
};

/********************************************************************************************
 *                                EChar                                                     *
 ********************************************************************************************/

class EChar : public ECUnit, public ECBChar
{
/* Constructeur/Destructeur */
public:

	CHAR_EMPTY_CONSTRUCTOR(EChar), ECUnit(3) {}

	CHAR_CONSTRUCTOR(EChar), ECUnit(3)
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

class ECArmy : public ECUnit, public ECBArmy
{
/* Constructeur/Destructeur */
public:

  ARMY_EMPTY_CONSTRUCTOR(ECArmy), ECUnit(2) {}

	ARMY_CONSTRUCTOR(ECArmy), ECUnit(2)
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
