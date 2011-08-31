/* src/gui/TMemo.cpp - TMemo GUI
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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
#include "Memo.h"
#include "Resources.h"
#include "tools/Maths.h"
#include <algorithm>
#include <SDL.h>
#include <assert.h>

TMemo::TMemo (Font* f, int _x, int _y, uint _width, uint _height, uint max_items, bool _show_background)
  : TComponent(_x, _y, _width, _height), show_background(_show_background), font(f), shadowed(false)
{
  height_item = f->GetHeight();
  first_visible_item = 0;
  nb_visible_items = 0;
  visible_height = 0;
  maxitems = max_items;
}

TMemo::~TMemo()
{
	for(std::vector<TLabel*>::iterator it = m_items.begin(); it != m_items.end(); ++it)
		delete *it;
}

void TMemo::Init()
{
	MyComponent(&m_up);
	MyComponent(&m_down);
	// Load images
	m_up.SetImage (new ECSprite(Resources::UpButton(), Window()));
	m_down.SetImage (new ECSprite(Resources::DownButton(), Window()));

	nb_visible_items_max = Height()/height_item;
	if (nb_visible_items_max < nb_visible_items)
		nb_visible_items = nb_visible_items_max;

	first_visible_item = m_items.size() - nb_visible_items;

	Rectanglei r(0, 0, Width(), show_background ? Height() : height_item);

	background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, Width(), show_background ? Height() : height_item,
					32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
	background.FillRect(r, show_background ? background.MapColor(BoxColor) : background.MapColor(BoxShadowColor));
	if (show_background)
		background.RectangleColor(r, white_color);

}

bool TMemo::Clic (const Point2i& mouse, int button)
{
  if(Mouse(mouse) == false) return false;


  if (m_items.size() > nb_visible_items_max)
  {
    if (button == SDL_BUTTON_WHEELDOWN || m_down.Test(mouse))
    {
      // bottom button
      if ( m_items.size() - first_visible_item > nb_visible_items_max ) first_visible_item++ ;
      return true;
    }


    if (button == SDL_BUTTON_WHEELUP || m_up.Test(mouse))
    {
      // top button
      if (first_visible_item > 0) first_visible_item-- ;
      return true;
    }
  }

  return false;
}

void TMemo::Draw (const Point2i& mouse)
{
	if(show_background && !background.IsNull())
		Window()->Blit(background, position);

	uint i=0;
	for(std::vector<TLabel*>::iterator it = m_items.begin()+first_visible_item;
	    i < nb_visible_items && it != m_items.end(); i++, ++it)
	{
		if(!show_background && !shadowed)
			Window()->Blit(background, Point2i(X(), Y()+i*height_item));
		(*it)->SetXY(X()+5, Y()+i*height_item);
		(*it)->Draw(mouse);
	}

	// buttons for listbox with more items than visible
	if (m_items.size() > nb_visible_items_max)
	{
		m_up.SetXY(X()+Width()-12, Y()+2);
		m_down.SetXY(X()+Width()-12, Y()+Height()-m_down.Height()-2);

		m_up.Draw (mouse);
		m_down.Draw (mouse);
	}
}

void TMemo::AddItem (const std::string &label, Color _color)
{
	const char *_s = label.c_str();
	/* On parse le message pour le découper en différentes lignes */
	std::string s;

	int marge = Width() < 200 ? Width()/4 : 100;

	while(1)
	{
		int size = font->GetWidth(s);
		if(*_s == '\n' || ((size > Width()-marge) && *_s == ' ') || ((size+20) > Width()) || !(*_s))
		{
			/* Suppression du premier element */
			if(maxitems && m_items.size() >= maxitems)
			{
				delete m_items.front();
				m_items.erase(m_items.begin());
			}
			else if (m_items.size() >= nb_visible_items_max) first_visible_item++;

			// Push item
			if((size+20) > (Width())) s += "-"; // On a tronqué on rajoute un indicateur

			TLabel* label = new TLabel(X(),Y(), s, _color, font, shadowed);
			MyComponent(label);
			m_items.push_back (label);

			nb_visible_items = m_items.size();
			if (nb_visible_items_max < nb_visible_items)
				nb_visible_items = nb_visible_items_max;

			visible_height = nb_visible_items*height_item;
			if (Height() < visible_height)  visible_height = Height();
			s.clear();

			if(*_s == '\n')
				_s++; /* Seulement une fois, pour retourner à la ligne si il y en a un autre */
			while(*_s == ' ') _s++;
			if(!*_s) break;
		}
		else
			s += *_s++;
	}
	SetWantRedraw();
}

void TMemo::RemoveItem (uint index)
{
	assert (index < m_items.size());
	delete *(m_items.begin() + index);
	m_items.erase( m_items.begin() + index );

	nb_visible_items = m_items.size();
	if( nb_visible_items_max < nb_visible_items )
		nb_visible_items = nb_visible_items_max;

	visible_height = nb_visible_items*height_item;
	if (Height() < visible_height)  visible_height = Height();

	SetWantRedraw();
}

void TMemo::ClearItems()
{
	for(std::vector<TLabel*>::iterator it = m_items.begin(); it != m_items.end(); ++it)
		delete *it;
	m_items.clear();
	first_visible_item = 0;
	visible_height = 0;
	nb_visible_items = 0;
	SetWantRedraw();
}
