/* src/gui/SpinEdit.h - Header of SpinEdit.cpp
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

#ifndef EC_SPINEDIT_H
#define EC_SPINEDIT_H

#define SPINEDIT_HEIGHT 15
#include "Component.h"
#include "Boutton.h"
#include "Label.h"
#include "tools/Font.h"
#include <string>
#include <SDL.h>

class TSpinEdit : public TComponent
{
/* Constructeur/Destructeur */
public:

	TSpinEdit(std::string label, uint _x, uint _y, uint _width, int _min, int _max, uint _step = 1, int _defvalue = 0);
	~TSpinEdit();

/* Methodes */
public:

	void Init();

	void Draw (uint mouse_x, uint mouse_y);

	bool Clic (uint mouse_x, uint mouse_y);

	void SetColorFont(SDL_Color new_color, Font* new_font);

/* Attributs */
public:

	/* Valeur du champ */
	void SetValue(int _value);
	int Value() { return value; }

/* Variables privées */
protected:
	SDL_Surface *background;
	TButton *m_plus, *m_minus;
	TLabel *txt_label, *txt_value;
	int value, min, max;
	uint step;
	uint visible_len;
	std::string label;

	SDL_Color color;
	Font *font;
};

#endif /* EC_SPINEDIT_H */
