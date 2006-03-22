/* src/gui/ChildForm.h - Header of ChildForm.cpp
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
 * $Id$
 */

#ifndef EC_CHILDFORM_H
#define EC_CHILDFORM_H

#include "Component.h"
#include "tools/Images.h"

class TChildForm : public TComponent
{
/* Constructeur/Destructeur */
public:

	TChildForm(int _x, int _y, uint _w, uint _h);
	virtual ~TChildForm() {}

/* Methodes */
public:

	/* Dessine chaques composants */
	void Draw(int x, int y);

	void Init() {}

/* Attributs */
public:

	/** Set background picture */
	void SetBackground(ECImage *image);

	virtual void SetXY(int x, int y);

/* Variables protégées */
protected:

	ECImage *background;

	/** Add a component */
	template<typename T>
	T* AddComponent(T* comp)
	{
		composants.push_back(comp);
		comp->SetXY(comp->X()+x, comp->Y()+y);
		comp->Init();
		return comp;
	}

/* Variables privées */
private:
	std::vector<TComponent*> composants;
};

#endif /* EC_CHILDFORM_H */
