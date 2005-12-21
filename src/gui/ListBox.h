/* src/gui/ListBox.h - Header of ListBox.cpp
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

#ifndef EC_LISTBOX_H
#define EC_LISTBOX_H

#include <vector>
#include <string>
#include <list>

#include "Boutton.h"
#include "Component.h"

struct SDL_Surface;
struct SDL_Color;

typedef struct s_list_box_item_t{
    std::string label;
    std::string value;
    SDL_Color color;
    bool enabled;
  } list_box_item_t;


class TListBox : public TComponent
{
public:
  int selection_min; // Minimal number of selected items
  int selection_max; // Maximal number of selected items (-1 means no limit)
  typedef std::list<uint>::const_iterator selection_iterator;

private:

  uint visible_height;
  uint nb_visible_items, nb_visible_items_max;
  uint height_item;

  // what are the items ?
  uint first_visible_item;
  std::vector<list_box_item_t> m_items;

  //std::vector<ListBoxItem> m_items;
  std::list<uint> m_selection;

  // Buttons
  TButton m_up, m_down;

  SDL_Surface *cursorover_box;
  SDL_Surface *selected_box;
  SDL_Surface *background;

public:
  TListBox (uint _x, uint _y, uint _width, uint _height);
  ~TListBox();
  void Init ();

  void Draw (uint mouse_x, uint mouse_y);
  bool Clic (uint mouse_x, uint mouse_y);

  void AddItem (bool selected,
		const std::string &label,
		const std::string &value,
		SDL_Color _color, bool _enabled);
  void ClearItems();
  int MouseIsOnWitchItem (uint mouse_x, uint mouse_y);
  void Select (uint index);
  void Deselect (uint index);
  bool IsSelected (uint index);
  int GetSelectedItem (); /* retourne -1 si non sélectionné */
  const std::list<uint>& GetSelection() const;
  const std::string& ReadLabel (uint index) const;
  const std::string& ReadValue (uint index) const;
  bool Enabled(uint index);
};

#endif /* EC_LISTBOX_H */
