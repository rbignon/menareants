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

class TMap : public TComponent
{
/* Constructeur/Destructeur */
public:

	TMap(ECMap* _map)
		: TComponent(0,0), map(_map), x_min(0), y_min(0)
	{}

/* Methodes */
public:

	void Init();

	void Draw(int x, int y);

	ECEntity* TestEntity(int mouse_x, int mouse_y);
	ECase* TestCase(int mouse_x, int mouse_y);

	virtual void SetXY(int x, int y);

	void SetPosition(int _x, int _y, bool force = false);

	/* Centre sur un objet ou des coordonées (pixels) de la carte */
	void CenterTo(ECase* e) { CenterTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void CenterTo(ECEntity* e) { CenterTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void CenterTo(int _x, int _y);

	/* Fait un déplacement progressif */
	void ScrollTo(ECase* e) { ScrollTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void ScrollTo(ECEntity* e) { ScrollTo(e->Image()->X() - x, e->Image()->Y() - y); }
	void ScrollTo(int _x, int _y);

	/* Le point (0,0) de l'écran coincide avec ce point */
	void MoveTo(int _x, int _y);

/* Attributs */
public:

	int Xmin() { return x_min; }
	int Ymin() { return y_min; }
	void SetContraintes(int _x, int _y) { x_min = _x; y_min = _y; }

/* Variables protégées */
protected:
	ECMap* map;
	int x_min, y_min;
};

#endif /* EC_SHOWMAP_H */
