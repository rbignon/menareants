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
#include "ListBox.h"
#include "Main.h"
#include "Resources.h"
#include "tools/Maths.h"
#include <algorithm>
#include <SDL.h>

TListBox::TListBox (int _x, int _y, uint _width, uint _height)
  : TComponent(_x, _y, _width, _height)
{
  height_item = 15;
  first_visible_item = 0;
  nb_visible_items = 0;
  visible_height = 0;
  m_selection = -1;
  gray_disable = false;

  cursorover_box = NULL;
  selected_box = NULL;
  background = NULL;
}

TListBox::~TListBox()
{
   if ( cursorover_box)
     SDL_FreeSurface( cursorover_box);
   if ( selected_box)
     SDL_FreeSurface( selected_box);
   if ( background)
     SDL_FreeSurface( background);
}

void TListBox::Init()
{
  // Load images

  m_up.SetImage (new ECSprite(Resources::UpButton(), app.sdlwindow));
  m_down.SetImage (new ECSprite(Resources::DownButton(), app.sdlwindow));

  nb_visible_items_max = h/height_item;

  if ( cursorover_box)
    SDL_FreeSurface( cursorover_box);
  if ( selected_box)
    SDL_FreeSurface( selected_box);
  if ( background)
    SDL_FreeSurface( background);

  SDL_Rect r_item = {0,0,w,height_item};
  SDL_Rect r_back = {0,0,w,h};

  cursorover_box = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, height_item,
					 32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( cursorover_box, &r_item, SDL_MapRGBA( cursorover_box->format,0,0,255*6/10,255*4/10));

  selected_box = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, height_item,
					 32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( selected_box, &r_item, SDL_MapRGBA( selected_box->format,0,0,255*6/10,255*8/10));

  background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));

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

bool TListBox::Clic (int mouse_x, int mouse_y)
{
  // buttons for listbox with more items than visible
  if (m_items.size() > nb_visible_items_max)
  {
    if ( m_down.Test(mouse_x, mouse_y) )
    {
      // bottom button
      if ( m_items.size()-1 - first_visible_item > nb_visible_items_max ) first_visible_item++ ;
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
  if (IsSelected(item))
    Deselect (item);
  else
    Select (item);
  return true;
}

void TListBox::Draw (int mouse_x, int mouse_y)
{
  int item = MouseIsOnWitchItem(mouse_x, mouse_y);

  // blit a surface as SDL_FillRect don't alpha blit a rectangle
  SDL_Rect r_back = {x,y,w,h};
  SDL_BlitSurface( background, NULL, app.sdlwindow, &r_back);

  for (uint i=0; i < nb_visible_items; i++)
  {
	// blit surfaces as SDL_FillRect don't alpha blit a rectangle
	if(enabled && m_items[i+first_visible_item].enabled)
	{
		if ( i+first_visible_item == uint(item))
		{
			SDL_Rect r = {x+1, y+i*height_item+1, w-2, height_item-2};
			SDL_BlitSurface( cursorover_box, NULL, app.sdlwindow, &r);
		}
		else if ( IsSelected(i+first_visible_item))
		{
			SDL_Rect r = {x+1, y+i*height_item+1, w-2, height_item-2};
			SDL_BlitSurface( selected_box, NULL, app.sdlwindow, &r);
		}
	}
	if(!m_items[i+first_visible_item].label.empty())
		app.Font()->sm.WriteLeft(x+5,
			 y+i*height_item,
			 m_items[i+first_visible_item].label,
			 !m_items[i+first_visible_item].enabled && gray_disable ? gray_color :
			 IsSelected(i+first_visible_item) && enabled ? white_color : m_items[i+first_visible_item].color) ;
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

void TListBox::AddItem (bool selected,
		       const std::string &label,
		       const std::string &value, SDL_Color _color = black_color, bool enabled = true)
{
  uint pos = m_items.size();

  // Push item
  list_box_item_t item;
  item.label = label;
  item.value = value;
  item.color = _color;
  item.enabled = enabled;
  m_items.push_back (item);

  // Select it if selected
  if (selected) Select (pos);

  nb_visible_items = m_items.size();
  if (nb_visible_items_max < nb_visible_items)
    nb_visible_items = nb_visible_items_max;

  visible_height = nb_visible_items*height_item;
  if (h < visible_height)  visible_height = h;

}

void TListBox::ClearItems()
{
	m_items.clear();
	m_selection = -1;
	height_item = 15;
	first_visible_item = 0;
	visible_height = 0;
	nb_visible_items = 0;
}

void TListBox::Select (uint index)
{
	if(!m_items[index].enabled) return;

	m_selection = index;
}

//-----------------------------------------------------------------------------

void TListBox::Deselect (uint index)
{
	if (m_selection == (int)index)
		m_selection = -1;
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

//-----------------------------------------------------------------------------

const std::string& TListBox::ReadLabel (uint index) const
{
  assert (index < m_items.size());
  return m_items[index].label;
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
