/* src/gui/ComboBox.h - Header of ComboBox.cpp
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

#ifndef EC_COMBOBOX_H
#define EC_COMBOBOX_H

#include "ListBox.h"
#include "tools/Font.h"

class TComboBox : public TListBox
{
/* Constructeur/Destructeur */
public:
	TComboBox(Font* f, int _x, int _y, uint _width);

	virtual ~TComboBox();

/* Methodes */
public:

	virtual void Draw (int mouse_x, int mouse_y);
	virtual bool Clic (int mouse_x, int mouse_y);
	virtual void Init ();

/* Attributs */
public:

	bool Opened() const { return opened; }

	uint AddItem (bool selected,
	              const std::string &label,
	              const std::string &value,
	              SDL_Color _color = black_color, bool _enabled = true);
	void ClearItems();
	virtual void Deselect (uint index);
	virtual void Select(uint index);

/* Variables priv�es */
protected:
	TButton m_open;
	int real_y;
	SDL_Surface *edit_bg;
	bool opened;
	uint visible_len;
	void SetOpened(bool _o);
	void SetBackGround(uint _h);
	std::string chaine;
	uint COMBOBOX_HEIGHT;
};

#endif /* EC_COMBOBOX_H */
