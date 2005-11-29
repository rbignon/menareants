/* src/gui/Edit.cpp - Edit GUI
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
#include "Main.h"
#include "Edit.h"
#include <SDL.h>

Edit::Edit (uint _x, uint _y, uint _width, uint _maxlen)
  : x(_x), y(_y), width(_width), height(EDIT_HEIGHT)
{
  first_char = 0;
  maxlen = _maxlen;
  focus = false;
  chaine = "";

  visible_len = ((width) / small_font.GetWidth("A"));

  background = NULL;
}

Edit::~Edit()
{
   if ( background)
     SDL_FreeSurface( background);
}

void Edit::Init()
{
  if ( background)
    SDL_FreeSurface( background);

  SDL_Rect r_back = {0,0,width,height};

  background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, width, height,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));

}

bool Edit::Clic (uint mouse_x, uint mouse_y)
{
	if((x <= mouse_x) && (mouse_x <= x+width)
	  && (y <= mouse_y) && (mouse_y <= y+height))
		SetFocus();
	else
		DelFocus();

  return true;
}

void Edit::SetFocus()
{
	focus = true;
}

void Edit::DelFocus()
{
	focus = false;
}

void Edit::Display ()
{
  SDL_Rect r_back = {x,y,width,height};
  SDL_BlitSurface( background, NULL, app.sdlwindow, &r_back);

  if(!focus && chaine.empty()) return;

  if(chaine.size() > visible_len)
	small_font.WriteLeft(x+5, y,
			 chaine.substr(first_char, first_char+visible_len) + std::string(focus ? "_" : ""),
			 black_color);
  else
	small_font.WriteLeft(x+5, y, chaine + std::string(focus ? "_" : ""), black_color);
}

void Edit::PressKey(SDL_keysym key)
{
	if(!focus) return;

	if (key.sym == SDLK_BACKSPACE)
		chaine = std::string(chaine, 0, chaine.size()-1 );
	else if(maxlen && chaine.size() >= maxlen)
		return;
	else if(strchr(EDIT_CHARS, key.unicode))
	{
		char s[2];
		sprintf(s, "%c", key.unicode);
		chaine += s;
	}

	if(chaine.size() > visible_len) first_char = chaine.size() - visible_len;
}
