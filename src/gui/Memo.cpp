/* src/gui/TMemo.cpp - TMemo GUI
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
#include "Memo.h"
#include "Resources.h"
#include "tools/Maths.h"
#include <algorithm>
#include <SDL.h>

TMemo::TMemo (Font* f, int _x, int _y, uint _width, uint _height, uint max_items, bool _show_background)
  : TComponent(_x, _y, _width, _height), show_background(_show_background), font(f)
{
  height_item = f->GetHeight();
  first_visible_item = 0;
  nb_visible_items = 0;
  visible_height = 0;
  maxitems = max_items;

  background = NULL;
}

TMemo::~TMemo()
{
   if ( background)
     SDL_FreeSurface( background);
}

void TMemo::Init()
{
  MyComponent(&m_up);
  MyComponent(&m_down);
  // Load images
  m_up.SetImage (new ECSprite(Resources::UpButton(), Window()));
  m_down.SetImage (new ECSprite(Resources::DownButton(), Window()));

  nb_visible_items_max = h/height_item;

  if ( background)
    SDL_FreeSurface( background);

  if(!show_background) return;

  SDL_Rect r_back = {0,0,w,h};

  background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));

}

bool TMemo::Clic (int mouse_x, int mouse_y)
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

void TMemo::Draw (int mouse_x, int mouse_y)
{
	if(background)
	{
		SDL_Rect r_back = {x,y,w,h};
		SDL_BlitSurface( background, NULL, Window(), &r_back);
	}

	for (uint i=0; i < nb_visible_items; i++)
	{
		if(i+first_visible_item >= m_items.size()) continue;
		if(!m_items[i+first_visible_item].label.empty())
			font->WriteLeft(x+5,
				y+i*height_item,
				m_items[i+first_visible_item].label,
				m_items[i+first_visible_item].color) ;
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

void TMemo::AddItem (const std::string &label, SDL_Color _color)
{
	const char *_s = label.c_str();
	/* On parse le message pour le découper en différentes lignes */
	std::string s;

	int marge = Width() < 200 ? Width()/4 : 100;

	while(1)
	{
		uint size = font->GetWidth(s);
		if(*_s == '\n' || ((size > Width()-marge) && *_s == ' ') || ((size+20) > (Width())) || !(*_s))
		{
			/* Suppression du premier element */
			if(maxitems && m_items.size() >= maxitems) m_items.erase(m_items.begin());
			else if (m_items.size() >= nb_visible_items_max) first_visible_item++;

			// Push item
			memo_box_item_t item;
			if((size+20) > (Width())) s += "-"; // On a tronqué on rajoute un indicateur
			item.label = s;
			item.color = _color;
			m_items.push_back (item);

			nb_visible_items = m_items.size();
			if (nb_visible_items_max < nb_visible_items)
				nb_visible_items = nb_visible_items_max;

			visible_height = nb_visible_items*height_item;
			if (h < visible_height)  visible_height = h;
			s.clear();

			if(*_s == '\n')
				_s++; /* Seulement une fois, pour retourner à la ligne si il y en a un autre */
			while(*_s == ' ') _s++;
			if(!*_s) break;
		}
		else
			s += *_s++;
	}
}

void TMemo::RemoveItem (uint index)
{
	assert (index < m_items.size());
	m_items.erase( m_items.begin() + index );

	nb_visible_items = m_items.size();
	if( nb_visible_items_max < nb_visible_items )
		nb_visible_items = nb_visible_items_max;

	visible_height = nb_visible_items*height_item;
	if (h < visible_height)  visible_height = h;
}

void TMemo::ClearItems()
{
	m_items.clear();
	first_visible_item = 0;
	visible_height = 0;
	nb_visible_items = 0;
}
