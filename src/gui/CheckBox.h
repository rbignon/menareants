/* src/gui/CheckBox.h - Header of CheckBox.cpp
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
 * $Id: ComboBox.h 766 2006-06-17 11:41:58Z progs $
 */

#ifndef EC_CHECKBOX_H
#define EC_CHECKBOX_H

#include "Component.h"
#include "gui/Label.h"
#include "tools/Color.h"

class Font;

class TCheckBox : public TComponent
{
/* Constructeur/Destructeurs */
public:

	TCheckBox(Font* f, int x, int y, std::string txt, Color = black_color);
	~TCheckBox();

/* Methodes */
public:

	/** Initialisation */
	void Init ();

	/** Affiche */
	void Draw(int m_x, int m_y);
	bool Clic(int m_x, int m_y);

/* Attributs */
public:

	bool Checked() const { return checked; }
	void Check(bool b = true) { checked = b; }

	void SetXY (int _x, int _y);

/* Variables privées */
private:
	TLabel    text;
	ECSprite* image;
	bool      checked;
};

#endif /* EC_CHECKBOX_H */
