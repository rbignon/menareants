/* src/gui/ChildForm.h - Header of ChildForm.cpp
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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

class TMemo;

/** \warning When you use a TChildForm, you HAVE to use AddComponent() in Init() and not in the constructor !!! */
/* note pour mémoire: c'est parce que lors de la construction la variable Windows() n'est pas encore affectée, alors
 *                    que AddComponent() va la transmettre. Donc créer les composants dans Init() permet que nous meme
 *                    nous soyons bien initialisés par notre Parent
 */
class TChildForm : public TComponent
{
/* Constructeur/Destructeur */
public:

	TChildForm(int _x, int _y, uint _w, uint _h);
	virtual ~TChildForm() { Clear(); }

/* Methodes */
public:

	/* Dessine chaques composants */
	void Draw(const Point2i&);

	/** Use this function to init your components with AddComponent() */
	virtual void Init() = 0;

	virtual bool Clic(const Point2i&, int button);

	virtual void ClicUp(const Point2i&, int button);

	void Clear();

/* Attributs */
public:

	/** Set background picture */
	void SetBackground(ECImage *image);

	virtual void SetXY(int x, int y);

	virtual void DelFocus();

	virtual void PressKey(SDL_keysym);

	virtual bool WantRedraw() const { return true; }

	virtual bool IsHint(const Point2i& pos) const { return HaveHint(); }

/* Variables protégées */
protected:

	/** Add a component */
	template<typename T>
	T* AddComponent(T* comp)
	{
		composants.push_back(comp);
		comp->SetParent(this);
		comp->SetWindow(Window());
		comp->SetXY(comp->X() + X(), comp->Y() + Y());
		comp->Init();
		return comp;
	}

	void SetFocusOrder(bool s = true) { focus_order = s; }
	bool FocusOrder() { return focus_order; }

/* Variables privées */
private:
	std::vector<TComponent*> composants;
	ECImage *background;
	bool focus_order;
	Point2i lastmpos;
};

#endif /* EC_CHILDFORM_H */
