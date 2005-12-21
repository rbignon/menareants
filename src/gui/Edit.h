/* src/gui/Edit.h - Header of Edit.cpp
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
 *
 * $Id$
 */

#ifndef EC_EDIT_H
#define EC_EDIT_H

#include <vector>
#include <string>
#include <SDL.h>

#include "Component.h"

#define EDIT_HEIGHT 15
#define EDIT_CHARS "azertyuiopmlkjhgfdsqwxcvbnAZERTYUIOPMLKJHGFDSQWXCVBN123456789 .+-*/,;:!?()[]={}'\"&<>"

struct SDL_Surface;

class TEdit : public TComponent
{
/* Constructeur/Deconstructeur */
public:

	TEdit(uint _x, uint _y, uint _width, uint _maxlen = 0);
	~TEdit();

/* Methodes */
public:

	/* Initialisation */
	void Init ();

	/* Click */
	bool Clic (uint mouse_x, uint mouse_y);

	/* Affiche */
	void Draw(uint m_x, uint m_y);

	/* Une touche a été pressée */
	void PressKey(SDL_keysym);

/* Attributs */
public:

	/* Le composant a le focus ? */
	bool Focused() { return focus; }
	void SetFocus();
	void DelFocus();

	/* Chaine */
	const std::string& GetString() const { return chaine; }
	void ClearString() { chaine = ""; }

/* Variables privées */
private:
	SDL_Surface *background;
	uint maxlen;
	uint visible_len;
	bool focus;
	uint first_char;
	std::string chaine;
};

#endif /* EC_EDIT_H */
