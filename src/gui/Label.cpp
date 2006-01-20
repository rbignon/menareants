/* src/gui/Label.cpp - Show a litle text
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
 * $Id$
 */

#include "Label.h"
#include "Main.h"

TLabel::TLabel(unsigned int x, unsigned int y, std::string new_txt, SDL_Color new_color, Font* new_font)
	: TComponent(x, y)
{
  assert(new_font!=NULL);
  color = new_color;
  caption = "";
  font = new_font;
  surf = NULL;
  SetCaption(new_txt);
}

TLabel::~TLabel()
{
  if(surf!=NULL)
    SDL_FreeSurface(surf);
}

void TLabel::SetCaption (std::string new_txt)
{
  if(caption == new_txt)
    return;

  caption = new_txt;

  if (surf != NULL)
  {
    SDL_FreeSurface(surf);
  }

  //Doesn't work.. don't know why...
/*  SDL_Surface* tmp = font->Render(new_txt,color);
  surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, tmp->w, tmp->h, 32,
                                0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
  SDL_Rect dst_rect = {0,0,tmp->w,tmp->h};
  SDL_BlitSurface(tmp, NULL, surf, &dst_rect);*/

  surf = TTF_RenderUTF8_Blended(font->m_font, caption.c_str(),color);
}

void TLabel::Draw(unsigned int m_x, unsigned int m_y)
{
	if(!surf) return; /* Possible (mais bizare). Par exemple un SpinEdit sans texte */

	SDL_Rect dst_rect;
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = surf->w;
	dst_rect.h = surf->h;

	SDL_BlitSurface(surf,NULL,app.sdlwindow, &dst_rect);
}
