/* src/gui/Component.h - Header of Component.cpp
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

#ifndef EC_COMPONENT_H
#define EC_COMPONENT_H

typedef unsigned int   uint;

class TComponent
{
/* Constructeur/Deconstructeur */
public:

	TComponent() : visible(true), enabled(true)
	{}
	TComponent(uint _x, uint _y)
		: x(_x), y(_y), visible(true), enabled(true)
	{}
	TComponent(uint _x, uint _y, uint _w, uint _h)
		: x(_x), y(_y), h(_h), w(_w), visible(true), enabled(true)
	{}

	virtual ~TComponent() {}

/* Méthodes */
public:

	/* Dessine l'objet */
	virtual void Draw(unsigned int souris_x, unsigned int souris_y) = 0;

	/* Fonction d'initialisation */
	virtual void Init() = 0;

/* Attributs */
public:

	/* Obtient la position, la hauteur ou la largeur */
	unsigned int GetX() const;
	unsigned int GetY() const;
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

	/* Définie la position, la hauteur ou la largeur */
	void SetXY (uint _x, uint _y);
	void SetHeight (uint _h);
	void SetWidth (uint _w);

	/* Visibilité */
	bool Visible() const { return visible; }
	void Show() { visible = true; }
	void Hide() { visible = false; }

	/* Actif */
	bool Enabled() { return enabled; }
	void SetEnabled(bool _en = true) { enabled = _en; }

/* Variables privées */
protected:
	uint x, y;
	uint h, w;
	bool visible;
	bool enabled;
};

#endif /* EC_COMPONENT_H */
