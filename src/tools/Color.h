/* src/tools/Color.h - Header of Color.cpp
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
/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Handle a SDL_Surface.
 *****************************************************************************/

#ifndef EC_COLOR_H
#define EC_COLOR_H

#include <SDL.h>

class Color
{
private:
	Uint8 red;
	Uint8 green;
	Uint8 blue;
	Uint8 alpha;

public:
	Color();
	Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	bool operator==(const Color &color) const;

	void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	Uint8 GetRed() const;
	Uint8 GetGreen() const;
	Uint8 GetBlue() const;
	Uint8 GetAlpha() const;

	SDL_Color GetSDLColor() const;
};

const Color white_color(0xFF, 0xFF, 0xFF, 0xff);
const Color black_color(0x00, 0x00, 0x00, 0xff);
const Color red_color(0xFF, 0x00, 0x04, 0xff);
const Color gray_color(0x56, 0x56, 0x56, 0xff);
const Color green_color(0x00, 0x30, 0x00, 0xff);
const Color brown_color(0x6A, 0x4C, 0x3C, 0xff);
const Color blue_color(0x00, 0x00, 0xFF, 0xff);
const Color orange_color(0xff, 0x8c, 0x45, 0xff);
const Color violet_color(0x44, 0x21, 0x4f, 0xff);
const Color cyan_color(0x85, 0xf7, 0xff, 0xff);

const Color fgreen_color(0x00, 0xaf, 0x00, 0xff); ///< \note on devrait inverser avec green_color mais bon
const Color fred_color(0x80, 0x00, 0x02, 0xff);
const Color fwhite_color(0xcb, 0xcb, 0xcb, 0xff);
const Color fbrown_color(0x3b, 0x2b, 0x1e, 0xff);
const Color fblue_color(0x0e, 0x0e, 0x55, 0xff);

#endif /* EC_COLOR_H */
