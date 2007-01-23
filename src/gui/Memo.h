/* src/gui/Memo.h - Header of Memo.cpp
 *
 * Copyright (C) 2005,2007 Romain Bignon  <Progs@headfucking.net>
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

#ifndef EC_MEMO_H
#define EC_MEMO_H

#include <vector>
#include <string>

#include "Component.h"
#include "Boutton.h"
#include "gui/Label.h"

struct ECImage;
struct Color;
class Font;

/** This is a component derived of TComponent whose show a box with text lines */
class TMemo : public TComponent
{
private:
  // for the placement
  int visible_height;
  uint nb_visible_items, nb_visible_items_max;
  uint height_item;
  uint maxitems;

  // what are the items ?
  uint first_visible_item;
  std::vector<TLabel*> m_items;

  // Buttons
  TButton m_up, m_down;

  bool show_background;

  ECImage background;

  Font* font;

  bool shadowed;

public:
  TMemo (Font* font, int _x, int _y, uint _width, uint _height, uint max_items = 0, bool show_background = true);
  ~TMemo();
  void Init ();
  void Draw (const Point2i& mouse);
  bool Clic (const Point2i& mouse, int button);
  void AddItem (const std::string &label, Color _color = black_color);
  void ScrollUp() { first_visible_item = 0; };
  void RemoveItem (uint index);
  uint NbItems() const { return m_items.size(); }
  void ClearItems();
  bool Empty() const { return m_items.empty(); }
  void SetShadowed(bool b = true) { shadowed = b; }
};

#endif /* EC_MEMO_H */
