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

TLabel::TLabel(int x, int y, std::string new_txt, SDL_Color new_color, Font* new_font)
	: TComponent(x, y)
{
	assert(new_font!=NULL);
	color = new_color;
	caption = "";
	surf = 0;
	font = new_font;
	SetCaption(new_txt);
}

TLabel::~TLabel()
{
	if(surf)
		SDL_FreeSurface(surf);
}

void TLabel::SetCaption (std::string new_txt)
{
	if(caption == new_txt)
		return;
	
	caption = new_txt;
	Reinit();
}

void TLabel::SetColor(SDL_Color new_color)
{
	color = new_color;
	Reinit();
}

void TLabel::Reinit()
{
	if (surf != NULL)
	{
		SDL_FreeSurface(surf);
	}

	if(caption.empty()) return;

	surf = TTF_RenderText_Blended(&(font->GetTTF()), caption.c_str(),color);
	SetHeight(font->GetHeight());
	SetWidth(font->GetWidth(caption));
}

void TLabel::Draw(int m_x, int m_y)
{
	if(!surf) return; /* Possible (mais bizare). Par exemple un SpinEdit sans texte */

	SDL_Rect dst_rect;
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = surf->w;
	dst_rect.h = surf->h;

	SDL_BlitSurface(surf,NULL,Window(), &dst_rect);
}
