/* src/gui/ListBox.h - Header of ListBox.cpp
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

/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * Liste de choix.
 *****************************************************************************/

#ifndef LIST_BOX_H
#define LIST_BOX_H

#include <string>
#include <vector>
#include "gui/Component.h"
#include "gui/Boutton.h"
#include "gui/Label.h"
#include "tools/Font.h"

class TListBox;

class TListBoxItem : public TLabel
{
private:
  std::string value;
  std::string name;

public:
  TListBoxItem(const std::string& _name, const std::string& _label, Font& _font, const std::string& value,
	      const Color& color = white_color);

  const std::string& Label() const;
  const std::string& Value() const;
  const std::string& Name() const;
};

class TListBox : public TComponent
{
/* Constructeurs */
public:
	TListBox (const Rectanglei &rect, bool always_one_selected_b = true);
	~TListBox();

	typedef void (*OnChangeFunc) (TListBox*);

/* Methodes */
public:

	void Init();

	void Draw(const Point2i &mousePosition);

	bool Clic(const Point2i &mousePosition, int button);

	void ClicUp (const Point2i& pos, int button);

	TListBoxItem* AddItem(bool selected, const std::string &label,
	                      const std::string &value,
	                      const Color& color = white_color, bool enabled = true,
	                      Font& font = *Font::GetInstance(Font::Small), const std::string& name = "");

	void Sort();

	int MouseIsOnWhichItem(const Point2i &mousePosition) const;

	virtual void Select(uint index);

	void Select(const std::string& val);

	virtual void Deselect();

	void ClearItems();

	void ScrollTo(uint id);
	void ScrollTo(TListBoxItem* item);

/* Attributs */
public:

	TListBoxItem* SelectedItem() const;

	int Selected() const;

	TListBoxItem* Item(uint r) const;

	void RemoveSelected();

	void SetEnabled(bool _en = true);

	const std::string& ReadLabel() const;
	const std::string& ReadValue() const;
	const std::string& ReadValue(int index) const;
	const std::string& ReadName(int index) const;

	uint Size() const;

	bool Empty() const { return m_items.empty(); }

	void SetOnChange(OnChangeFunc on) { on_change = on; }

	void SetNoItemHint(bool b = true) { no_item_hint = b; }
	bool NoItemHint() const { return no_item_hint; }

	void SetBackgroundColor(Color c) { box_color = c; }

	void SetGrayDisable(bool b  = true) { gray_disable = b; }

	Rectanglei ScrollBarPos() const;

/* Variables privées */
private:
	bool always_one_selected;
	bool no_item_hint;
	bool scrolling;
	Point2i scroll_point;

protected:
	// what are the items ?
	uint first_visible_item;
	int selected_item;
	std::vector<TListBoxItem*> m_items;

	Color box_color;

	// Buttons
	TButton m_up, m_down;

	OnChangeFunc on_change;

	bool gray_disable;
};

#endif
