/* src/gui/TMemo.cpp - TMemo GUI
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

#include "tools/Font.h"
#include "Memo.h"
#include "Main.h"
#include "Resources.h"
#include "tools/Maths.h"
#include <algorithm>
#include <SDL.h>

TMemo::TMemo (uint _x, uint _y, uint _width, uint _height, uint max_items = 0)
  : x(_x), y(_y), width(_width), height(_height)
{
  height_item = 15;
  first_visible_item = 0;
  nb_visible_items_max = height/height_item;
  nb_visible_items = 0;
  visible_height = 0;
  maxitems = max_items;

  maxlen = ((width) / small_font.GetWidth("A"));

  background = NULL;
}

TMemo::~TMemo()
{
   if ( background)
     SDL_FreeSurface( background);
}

void TMemo::Init()
{
  // Load images
  m_up.SetImage (new ECSprite(Resources::UpButton(), app.sdlwindow));
  m_down.SetImage (new ECSprite(Resources::DownButton(), app.sdlwindow));

  if ( background)
    SDL_FreeSurface( background);

  SDL_Rect r_back = {0,0,width,height};

  background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, width, height,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));

}

bool TMemo::Clic (uint mouse_x, uint mouse_y)
{
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

  return false;
}

void TMemo::Display (uint mouse_x, uint mouse_y)
{
  // blit a surface as SDL_FillRect don't alpha blit a rectangle
  SDL_Rect r_back = {x,y,width,height};
  SDL_BlitSurface( background, NULL, app.sdlwindow, &r_back);

  for (uint i=0; i < nb_visible_items; i++)
  {
	small_font.WriteLeft(x+5,
			 y+i*height_item,
			 m_items[i+first_visible_item].label,
			 m_items[i+first_visible_item].color) ;
  }

  // buttons for listbox with more items than visible
  if (m_items.size() > nb_visible_items_max)
  {
    m_up.SetPos(x+width-12, y+2);
    m_down.SetPos(x+width-12, y+height-7);

    m_up.Draw (mouse_x, mouse_y);
    m_down.Draw (mouse_x, mouse_y);
  }
}

void TMemo::AddItem (const std::string &_label, SDL_Color _color = black_color)
{
  std::string label = _label;
  bool theend = false;
  while(!theend)
  {
	std::string toadd;
	if(label.size() > maxlen)
	{
		toadd = label.substr(0, maxlen);
		label = label.substr(maxlen);
	}
	else
	{
		toadd = label;
		theend = true;
	}

	/* Suppression du premier element */
	if(maxitems && m_items.size() >= maxitems) m_items.erase(m_items.begin());
	else if (m_items.size() >= nb_visible_items_max) first_visible_item++;

	// Push item
	memo_box_item_t item;
	item.label = toadd;
	item.color = _color;
	m_items.push_back (item);

	nb_visible_items = m_items.size();
	if (nb_visible_items_max < nb_visible_items)
		nb_visible_items = nb_visible_items_max;

	visible_height = nb_visible_items*height_item;
	if (height < visible_height)  visible_height = height;

  }

}

void TMemo::ClearItems()
{
	m_items.clear();
}

void TMemo::SetXY (uint px, uint py) { x = px; y = py; }
