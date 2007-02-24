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
#include "tools/Color.h"

class ECImage;
class Font;

class TComboBox : public TListBox
{
/* Constructeur/Destructeur */
public:
	TComboBox(Font* f, int _x, int _y, uint _width);

	virtual ~TComboBox() {}

/* Methodes */
public:

	virtual void Draw (const Point2i&);
	virtual bool Clic (const Point2i&, int button);
	virtual void Init ();

/* Attributs */
public:

	bool Opened() const { return opened; }

	TListBoxItem* AddItem (bool selected,
	                       const std::string &label,
	                       const std::string &value,
	                       const Color& _color = black_color,
	                       bool _enabled = true,
	                       Font& font = *Font::GetInstance(Font::Small),
	                       const std::string& name = "");
	void ClearItems();
	virtual void Deselect (uint index);
	virtual void Select(uint index);

	int RealY() const { return real_y; }

	virtual void SetXY (int x, int y);

	inline bool Intersect( const Rectanglei &r2 ) const
	{
		return Rectanglei::Intersect(r2) || Rectanglei(X(), real_y, edit_bg.GetWidth(), edit_bg.GetHeight()).Intersect(r2);
	}

/* Variables privées */
protected:
	TButton m_open;
	int real_y;
	ECImage edit_bg;
	bool opened;
	uint visible_len;
	void SetOpened(bool _o);
	TLabel chaine;
	uint COMBOBOX_HEIGHT;
	Font* font;

	int CalculateHeight();
};

#endif /* EC_COMBOBOX_H */
