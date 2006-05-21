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

#define EDIT_CHARS "azertyuiopmlkjhgfdsqwxcvbnAZERTYUIOPMLKJHGFDSQWXCVBN123456789 .+-*/,;:!?()[]={}'\"&<>"

struct SDL_Surface;
class Font;

/** This is a component whose show a box where user can type text */
class TEdit : public TComponent
{
/* Constructeur/Deconstructeur */
public:

	TEdit(Font* font, int _x, int _y, uint _width, uint _maxlen = 0, char* av = EDIT_CHARS,
	      bool show_background = true);
	~TEdit();

/* Methodes */
public:

	/* Initialisation */
	void Init ();

	/* Affiche */
	void Draw(int m_x, int m_y);

	void Redraw();

	/* Une touche a été pressée */
	void PressKey(SDL_keysym);

/* Attributs */
public:

	/* Chaine */
	void SetString(std::string s);
	const std::string& GetString() const { return chaine; }
	const std::string& Text() const { return chaine; }
	void ClearString() { chaine = ""; caret = 0; first_char = 0; have_redraw = true; }
	bool Empty() const { return chaine.empty(); }

	const char* AvailChars() const { return avail_chars; }
	void SetAvailChars(char* c) { avail_chars = c; }

	void SetMaxLen(uint m) { maxlen = m; }

	void SetFocus();
	void DelFocus();

/* Variables privées */
private:
	SDL_Surface *background;
	SDL_Surface *edit;
	uint maxlen;
	uint visible_len;
	uint first_char;
	std::string chaine;
	bool show_background;
	uint caret;
	char* avail_chars;
	bool have_redraw;
	uint EDIT_HEIGHT;
	Font* font;
};

#endif /* EC_EDIT_H */
