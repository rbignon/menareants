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

TLabel::TLabel(const TLabel& label)
	: TComponent(label.x, label.y), surf(0), background(0), font(label.font), color(label.color),
	  shadowed(label.shadowed), bg_offset(label.bg_offset)
{
	SetParent(label.Parent());
	SetWindow(label.Window());
	caption = "";
	SetCaption(label.caption);
}

TLabel::TLabel(int x, int y, std::string new_txt, SDL_Color new_color, Font* new_font, bool _shadowed)
	: TComponent(x, y), surf(0), background(0), font(new_font), color(new_color), shadowed(_shadowed), bg_offset(0)
{
	assert(new_font!=NULL);
	caption = "";
	if(shadowed)
	{
		int width = font->GetWidth("x");
		bg_offset = (unsigned int)width/8; // shadow offset = 0.125ex
		if (bg_offset < 1) bg_offset = 1;
	}

	SetCaption(new_txt);
}

TLabel::~TLabel()
{
	if(surf)
		SDL_FreeSurface(surf);
	if(background)
		SDL_FreeSurface(background);
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
		surf = 0;
	}
	if(background)
	{
		SDL_FreeSurface(background);
		background = 0;
	}

	if(caption.empty()) return;

	surf = TTF_RenderText_Blended(&(font->GetTTF()), caption.c_str(),color);

	if(shadowed)
		background = TTF_RenderText_Blended(&(font->GetTTF()), caption.c_str(), white_color);

	SetHeight(surf->h);
	SetWidth(surf->w);
}

void TLabel::Draw(int m_x, int m_y)
{
	if(!surf || caption.empty()) return; /* Possible (mais bizare). Par exemple un SpinEdit sans texte */

	assert(Window());

	SDL_Rect dst_rect;
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = surf->w;
	dst_rect.h = surf->h;

	if(shadowed && background)
	{
		SDL_Rect shad_rect = {x+bg_offset, y+bg_offset, background->w, background->h};

		SDL_BlitSurface(background,NULL, Window(), &shad_rect);
	}
	SDL_BlitSurface(surf,NULL,Window(), &dst_rect);
}
