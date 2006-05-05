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
class Font;

/** An item of TListBox */
typedef struct s_list_box_item_t{
    std::string label;
    std::string value;
    std::string hint;
    SDL_Color color;
    bool enabled;
  } list_box_item_t;

/** This is a component based on TComponent and whose show a list of items */
class TListBox : public TComponent
{
/* Constructeur/Destructeur */
public:
	TListBox (Font *font, int _x, int _y, uint _width, uint _height);
	virtual ~TListBox();

/* Methodes */
public:

	virtual void Init ();
	
	virtual void Draw (int mouse_x, int mouse_y);
	virtual bool Clic (int mouse_x, int mouse_y);

/* Attributs */
public:

	uint AddItem (bool selected,
	              const std::string &label,
	              const std::string &value,
	              SDL_Color _color, bool _enabled);
	void RemoveItem (uint index);
	void SetItemHint(uint index, const char* Hint);
	void ClearItems();
	int MouseIsOnWitchItem (int mouse_x, int mouse_y);
	virtual void Select (uint index);
	virtual void Deselect (uint index);
	bool IsSelected (uint index);
	int GetSelectedItem (); /**< retourne -1 si non sélectionné */
	const std::string& ReadLabel (uint index) const;
	const std::string& ReadValue (uint index) const;
	bool EnabledItem(uint index);
	void SetEnabledItem(uint index, bool e = true);

/* Variables publiques */
public:

/* Variables protégées */
protected:

	uint visible_height;
	uint nb_visible_items, nb_visible_items_max;
	uint height_item;

	// what are the items ?
	uint first_visible_item;
	std::vector<list_box_item_t> m_items;
	int m_selection;
	bool gray_disable;

	// Buttons
	TButton m_up, m_down;

	SDL_Surface *cursorover_box;
	SDL_Surface *selected_box;
	SDL_Surface *background;

	Font* font;
};

#endif /* EC_LISTBOX_H */
