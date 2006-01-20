/* src/gui/Component.h - Header of Component.cpp
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

#ifndef EC_COMPONENT_H
#define EC_COMPONENT_H

#include <vector>

typedef unsigned int   uint;

/********************************************************************************************
 *                               TComponent                                                 *
 ********************************************************************************************/

class TComponent
{
/* Constructeur/Deconstructeur */
public:

	TComponent() : x(0), y(0), h(0), w(0), visible(true), enabled(true)
	{}
	TComponent(uint _x, uint _y)
		: x(_x), y(_y), h(0), w(0), visible(true), enabled(true)
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
	bool Enabled() const { return enabled; }
	void SetEnabled(bool _en = true) { enabled = _en; }

/* Variables privées */
protected:
	uint x, y;
	uint h, w;
	bool visible;
	bool enabled;
};

/********************************************************************************************
 *                                 TList                                                    *
 ********************************************************************************************/

class TList : public TComponent
{
/* Constructeur/Deconstructeur */
public:

	TList(uint _x, uint _y);

	~TList();

/* Méthodes */
public:

	/* Rajoute une ligne à la liste des composants */
	void AddLine(TComponent *);

	/* Supprime une ligne à la liste des composants */
	bool RemoveLine(TComponent *);

	/* Dessine tous les composants de la liste */
	void Draw(uint souris_x, uint souris_y);

	/* Initialisation non requise... (surcharge de fonction virtuelle) */
	void Init() {}

/* Attributs */
public:

	void SetXY (uint _x, uint _y);

	std::vector<TComponent*> GetList() const { return list; }

/* Variables privées */
private:

	/* Redéfinit x pour les composants de la liste et la hauteur du TList */
	void Rebuild();

	std::vector<TComponent*> list;
};

#endif /* EC_COMPONENT_H */
