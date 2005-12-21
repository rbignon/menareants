/* src/gui/TEdit.cpp - TEdit GUI
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

TEdit::TEdit (uint _x, uint _y, uint _width, uint _maxlen)
  : TComponent(_x, _y, _width, EDIT_HEIGHT)
{
  first_char = 0;
  maxlen = _maxlen;
  focus = false;
  chaine = "";

  background = NULL;
}

TEdit::~TEdit()
{
   if ( background)
     SDL_FreeSurface( background);
}

void TEdit::Init()
{
  if ( background)
    SDL_FreeSurface( background);

  visible_len = ((w) / small_font.GetWidth("A"));

  SDL_Rect r_back = {0,0,w,h};

  background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));

}

bool TEdit::Clic (uint mouse_x, uint mouse_y)
{
	if((x <= mouse_x) && (mouse_x <= x+w)
	  && (y <= mouse_y) && (mouse_y <= y+h))
		SetFocus();
	else
		DelFocus();

  return true;
}

void TEdit::SetFocus()
{
	focus = true;
}

void TEdit::DelFocus()
{
	focus = false;
}

void TEdit::Draw (uint m_x, uint m_y)
{
  SDL_Rect r_back = {x,y,w,h};
  SDL_BlitSurface( background, NULL, app.sdlwindow, &r_back);
  if(!focus && chaine.empty()) return;

  if(chaine.size() > visible_len)
	small_font.WriteLeft(x+5, y,
			 chaine.substr(first_char, first_char+visible_len) + std::string(focus ? "_" : ""),
			 black_color);
  else
	small_font.WriteLeft(x+5, y, chaine + std::string(focus ? "_" : ""), black_color);
}

void TEdit::PressKey(SDL_keysym key)
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
