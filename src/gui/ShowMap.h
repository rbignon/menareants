/* src/gui/ShowMap.h - Header of ShowMap.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#ifndef EC_SHOWMAP_H
#define EC_SHOWMAP_H

#include "Component.h"
#include "Map.h"

class ECImage;

class TMap : public TComponent
{
/* Constructeur/Destructeur */
public:

	TMap(ECMap* _map)
		: TComponent(0,0), map(_map), schema(false), x_min(0), y_min(0), create_entity(0),
		  have_brouillard(true), must_redraw(true), mutex(0)
	{}

/* Methodes */
public:

	void Init();

	void DrawFog(ECase* c);
	void Draw(int x, int y);

	ECEntity* TestEntity(int mouse_x, int mouse_y);
	ECase* TestCase(int mouse_x, int mouse_y);

	ECase* Pixel2Case(int x, int y);
	std::vector<ECase*> Rect2Case(int x, int y, uint w, uint h);

	void ToRedraw(ECSprite*);
	void ToRedraw(int x, int y);
	void ToRedraw(int x, int y, int w, int h);
	void ToRedraw(ECEntity*);
	void ToRedraw(ECase*);
	void ToRedraw(TComponent*);

	virtual void SetXY(int x, int y);

	void SetPosition(int _x, int _y, bool force = false);

	/* Centre sur un objet ou des coordon�es (pixels) de la carte */
	void CenterTo(ECase* e) { if(!e) return; CenterTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void CenterTo(ECEntity* e) { CenterTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void CenterTo(int _x, int _y);

	/* Fait un d�placement progressif */
	void ScrollTo(ECase* e) { ScrollTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void ScrollTo(ECEntity* e) { ScrollTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void ScrollTo(int _x, int _y);

	/* Le point (0,0) de l'�cran coincide avec ce point */
	void MoveTo(int _x, int _y);

/* Attributs */
public:

	int Xmin() const { return x_min; }
	int Ymin() const { return y_min; }
	void SetContraintes(int _x, int _y) { x_min = _x; y_min = _y; }

	bool Schema() const { return schema; }
	void SetSchema(bool s = true);
	void ToggleSchema() { SetSchema(!schema); }

	ECMap* Map() const { return map; }

	void SetCreateEntity(ECEntity* e) { create_entity = e; }
	ECEntity* CreateEntity() const { return create_entity; }

	bool HaveBrouillard() const { return have_brouillard; }
	void SetBrouillard(bool b = true) { have_brouillard = b; }

	bool MustRedraw() const { return must_redraw; }
	void SetMustRedraw(bool b = true);

	SDL_mutex* Mutex() const { return mutex; }
	void SetMutex(SDL_mutex* m) { mutex = m; }
	void LockScreen() const;
	void UnlockScreen() const;

	void AddAfterDraw(ECSprite* s) { after_draw.push_back(s); }
	void RemoveAfterDraw(ECSprite* s);

/* Variables priv�es */
private:
	ECMap* map;
	bool schema;
	int x_min, y_min;
	ECEntity* create_entity;
	bool have_brouillard;
	bool must_redraw;
	SDL_mutex* mutex;
	std::vector<ECSprite*> after_draw;
};

#endif /* EC_SHOWMAP_H */
