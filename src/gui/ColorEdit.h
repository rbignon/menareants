/* src/gui/ColorEdit.h - Header of ColorEdit.cpp
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

#ifndef EC_COLOREDIT_H
#define EC_COLOREDIT_H

#include "tools/Font.h"
#include "SpinEdit.h"
#include "Colors.h"

static SDL_Color *color_eq[] = {
	/* {COLOR_NONE  */          NULL,
	/* COLOR_BLACK  */          &black_color,
	/* COLOR_GRAY   */          &gray_color,
	/* COLOR_BLUE   */          &blue_color,
	/* COLOR_RED    */          &red_color,
	/* COLOR_GREEN  */          &green_color,
	/* COLOR_WHITE  */          &white_color,
	/* COLOR_BROWN  */          &brown_color,
	/* COLOR_ORANGE */          &orange_color,
	/* COLOR_MAX    */          NULL
};

class TColorEdit : public TSpinEdit
{
/* Constructor/Destructor */
public:
	TColorEdit(std::string label, uint _x, uint _y, uint _width, int _defvalue = 0);

	~TColorEdit();

/* Methodes */
public:

	void Init();

	void Draw (uint mouse_x, uint mouse_y);

/* Attributs */
public:

	bool SetValue(int _value, bool first = false);

/* Variables privées */
protected:
	ECImage *img;
	uint imgx;
};

#endif /* EC_COLOREDIT_H */
