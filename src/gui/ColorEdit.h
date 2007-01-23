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

class Color;

extern Color color_eq[];

class TColorEdit : public TSpinEdit
{
/* Constructor/Destructor */
public:
	TColorEdit(Font* font, std::string label, int _x, int _y, uint _width, int _defvalue = 0);

/* Methodes */
public:

	void Init();

	void Draw (const Point2i&);

/* Attributs */
public:

	bool SetValue(int _value, bool first = false);

	virtual void SetXY (int _x, int _y);

/* Variables privées */
protected:
	ECImage img;
	uint imgx;
};

#endif /* EC_COLOREDIT_H */
