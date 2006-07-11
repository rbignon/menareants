/* src/gui/TListBox.cpp - To draw a listbox
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
 *
 * $Id$
 */

#include "tools/Font.h"
#include "tools/Color.h"
#include "ListBox.h"
#include "Resources.h"
#include "tools/Maths.h"
#include <algorithm>
#include <SDL.h>

TListBox::TListBox (Font* f, int _x, int _y, uint _width, uint _height)
  : TComponent(_x, _y, _width, _height), no_item_hint(false), font(f), on_change(0)
{
  assert(font);
  height_item = f->GetHeight();
  first_visible_item = 0;
  nb_visible_items = 0;
  visible_height = 0;
  m_selection = -1;
  gray_disable = false;
}

void TListBox::Init()
{
  MyComponent(&m_up);
  MyComponent(&m_down);
  // Load images
  m_up.SetImage (new ECSprite(Resources::UpButton(), Window()));
  m_down.SetImage (new ECSprite(Resources::DownButton(), Window()));

  nb_visible_items_max = h/height_item;

  SDL_Rect r_item = {0,0,w,height_item};
  SDL_Rect r_back = {0,0,w,h};

  cursorover_box.SetImage(SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w-2, height_item,
					 32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
  cursorover_box.FillRect(r_item, cursorover_box.MapRGBA(0,0,255*6/10,255*4/10));

  selected_box.SetImage(SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w-2, height_item,
					 32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
  selected_box.FillRect(r_item, selected_box.MapRGBA(0,0,255*6/10,255*8/10));

  background.SetImage(SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
  background.FillRect(r_back, background.MapRGBA(255, 255, 255, 255*3/10));

}

int TListBox::MouseIsOnWitchItem (int mouse_x, int mouse_y)
{
  if ((mouse_x < x+1)
      || (mouse_y < y+1)
      || (int(y+1 + visible_height) < mouse_y)
      || (int(x + w) < mouse_x))
    return -1;

  int index = (mouse_y - y)/height_item;
  return BorneLong(index+first_visible_item, 0, m_items.size()-1);
}

void TListBox::ScrollTo(uint id)
{
	if(m_items.size() <= nb_visible_items_max) return;

	if( m_items.size() - id >= nb_visible_items_max)
		first_visible_item = id;
	else if(m_items.size()-1 - id < nb_visible_items_max)
		first_visible_item = m_items.size() - nb_visible_items_max;
}

bool TListBox::Clic (int mouse_x, int mouse_y)
{
  // buttons for listbox with more items than visible
  if (m_items.size() > nb_visible_items_max)
  {
    if ( m_down.Test(mouse_x, mouse_y) )
    {
      // bottom button
      if ( m_items.size() - first_visible_item > nb_visible_items_max ) first_visible_item++ ;
      return true;
    }


    if ( m_up.Test(mouse_x,mouse_y) )
    {
      // top button
      if (first_visible_item > 0) first_visible_item-- ;
      return true;
    }
  }

  int item = MouseIsOnWitchItem(mouse_x,mouse_y);
  if (item == -1 || !m_items[item].enabled) return false;
#if 0
  if (IsSelected(item))
    Deselect (item);
  else
#endif
  Select (item);
  if(on_change)
    (*on_change) (this);
  return true;
}

void TListBox::Draw (int mouse_x, int mouse_y)
{
	int item = MouseIsOnWitchItem(mouse_x, mouse_y);
	
	// blit a surface as SDL_FillRect don't alpha blit a rectangle
	SDL_Rect r_back = {x,y,w,h};
	Window()->Blit(background, &r_back);

	if(!NoItemHint())
		SetHint("");

	uint i=0;
	for(std::vector<list_box_item_t>::iterator it = m_items.begin()+first_visible_item;
	    i < nb_visible_items && it != m_items.end(); i++, ++it)
	{
		it->label.SetXY(x+5, y+i*height_item);
		if(Enabled() && it->enabled)
		{
			if ( i+first_visible_item == uint(item))
			{
				SDL_Rect r = {x+1, y+i*height_item+1, w-2, height_item-2};
				Window()->Blit(cursorover_box, &r);
				if(!NoItemHint())
					SetHint(it->label.Hint());
			}
			else if ( IsSelected(i+first_visible_item))
			{
				SDL_Rect r = {x+1, y+i*height_item+1, w-2, height_item-2};
				Window()->Blit(selected_box, &r);
			}
		}
		it->label.Draw(mouse_x, mouse_y);
	}

	// buttons for listbox with more items than visible
	if (m_items.size() > nb_visible_items_max)
	{
		m_up.SetXY(x+w-12, y+2);
		m_down.SetXY(x+w-12, y+h-7);
	
		m_up.Draw (mouse_x, mouse_y);
		m_down.Draw (mouse_x, mouse_y);
	}
}

uint TListBox::AddItem (bool selected,
		       const std::string &label,
		       const std::string &value, Color _color, bool enabled)
{
  uint pos = m_items.size();

  // Push item
  list_box_item_t item(font);
  item.label.SetXY(x,y);
  item.label.SetCaption(label);
  item.enabled = enabled;
  item.label.SetColor((!Enabled() || !enabled && gray_disable) ? gray_color : _color);
  MyComponent(&item.label);
  item.value = value;
  item.color = _color;
  m_items.push_back (item);

  // Select it if selected
  if (selected) Select (pos);

  nb_visible_items = m_items.size();
  if (nb_visible_items_max < nb_visible_items)
    nb_visible_items = nb_visible_items_max;

  visible_height = nb_visible_items*height_item;
  if (h < visible_height)  visible_height = h;
  return pos;
}

void TListBox::RemoveItem (uint index)
{
	assert (index < m_items.size());

	if(m_selection == (int)index)
		m_selection = -1;

	m_items.erase( m_items.begin() + index );

	nb_visible_items = m_items.size();
	if( nb_visible_items_max < nb_visible_items )
		nb_visible_items = nb_visible_items_max;

	visible_height = nb_visible_items*height_item;
	if (h < visible_height)  visible_height = h;
}

void TListBox::SetItemHint(uint index, std::string Hint)
{
  assert(index < m_items.size());

  m_items[index].label.SetHint(Hint);
}

void TListBox::ClearItems()
{
	m_items.clear();
	m_selection = -1;
	first_visible_item = 0;
	visible_height = 0;
	nb_visible_items = 0;
}

void TListBox::Select (uint index)
{
	if(!m_items[index].enabled) return;

	if(m_selection != -1)
		m_items[m_selection].label.SetColor(
		                         (!Enabled() || !m_items[m_selection].enabled && gray_disable) ? gray_color
		                                                                                       : m_items[index].color);

	m_selection = index;
	m_items[index].label.SetColor(white_color);
}

//-----------------------------------------------------------------------------

void TListBox::Deselect (uint index)
{
	if (m_selection == (int)index)
	{
		m_items[index].label.SetColor((!Enabled() || !m_items[index].enabled && gray_disable) ? gray_color :
		                                                                                        m_items[index].color);
		m_selection = -1;
	}
}

//-----------------------------------------------------------------------------

bool TListBox::IsSelected (uint index)
{
	return (m_selection == (int)index);
}

//-----------------------------------------------------------------------------

int TListBox::GetSelectedItem ()
{
  return m_selection;
}

void TListBox::SetEnabled(bool _en)
{
	TComponent::SetEnabled(_en);
	int i = 0;
	for(std::vector<list_box_item_t>::iterator it = m_items.begin(); it != m_items.end(); ++it, ++i)
		it->label.SetColor((!Enabled() || !it->enabled && gray_disable) ? gray_color : (m_selection == i) ? white_color :
		                                                                               it->color);
}

//-----------------------------------------------------------------------------

const std::string TListBox::ReadLabel (uint index) const
{
  assert (index < m_items.size());
  return m_items[index].label.Caption();
}
const std::string& TListBox::ReadValue (uint index) const
{
  assert (index < m_items.size());
  return m_items[index].value;
}

void TListBox::SetEnabledItem(uint index, bool e)
{
	assert (index < m_items.size());

	m_items[index].enabled = !m_items[index].enabled;
}

bool TListBox::EnabledItem (uint index)
{
  assert (index < m_items.size());
  return m_items[index].enabled;
}
