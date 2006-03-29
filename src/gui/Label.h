/* src/gui/Label.h - Header of Label.cpp
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

#ifndef EC_LABEL_H
#define EC_LABEL_H

#include <string>
#include <SDL.h>
#include "Component.h"
#include "tools/Font.h"

/** This is a component who shows a text */
class TLabel : public TComponent
{
/* Constructeur/Destructeur */
public:

	TLabel(int x, int y, std::string s, SDL_Color new_color, Font* new_font);
	~TLabel();

/* Methodes */
public:

	/** Draw */
	void Draw(int m_x, int m_y);

	/** Initialization. It is requiered to be defined by TComponent but it isn't necessary for TLabel */
	void Init() {}

/* Attributs */
public:

	void SetColor(SDL_Color new_color);
	void SetCaption(std::string text);
	std::string Caption() const { return caption; }

/* Variables privées */
protected:
	std::string caption;
	SDL_Surface* surf;
	Font* font;
	SDL_Color color;

	void Reinit();
};

#endif /* EC_LABEL_H */
