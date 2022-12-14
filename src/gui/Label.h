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
#include "Component.h"
#include "tools/Font.h"

class ECImage;

/** This is a component who shows a text */
class TLabel : public TComponent
{
/* Constructeur/Destructeur */
public:

	TLabel();
	TLabel(int x, int y, const std::string& s, Color new_color, Font* new_font, bool shadowed = false);
	TLabel(int y, const std::string& s, Color new_color, Font* new_font, bool shadowed = false);
	TLabel(const TLabel&);

/* Methodes */
public:

	/** Init */
	virtual void Init();

	/** Draw */
	void Draw(const Point2i&);

/* Attributs */
public:

	void SetGrayDisable(bool b = true) { gray_disable = b; }
	void SetColor(Color new_color);
	void SetFont(Font*);
	void SetFontColor(Font*, Color);
	void SetCaption(std::string text);
	const std::string& Caption() const { return caption; }
	bool Empty() const { return caption.empty(); }

	virtual void SetEnabled(bool _en = true);

/* Variables privées */
private:
	std::string caption;
	ECImage surf;
	ECImage background;
	Font* font;
	Color color;
	bool shadowed;
	uint bg_offset;
	bool auto_set;
	bool gray_disable;

	void Reinit();
};

#endif /* EC_LABEL_H */
