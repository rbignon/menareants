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
#include <assert.h>

TLabel::TLabel()
	: font(0), shadowed(0), bg_offset(0), auto_set(false)
{

}

TLabel::TLabel(const TLabel& label)
	: TComponent(label), font(label.font), color(label.color),
	  shadowed(label.shadowed), bg_offset(label.bg_offset), auto_set(label.auto_set)
{
	caption = "";
	SetCaption(label.caption);
}

TLabel::TLabel(int x, int y, std::string new_txt, Color new_color, Font* new_font, bool _shadowed)
	: TComponent(x, y), font(new_font), color(new_color), shadowed(_shadowed), bg_offset(0), auto_set(false)
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

TLabel::TLabel(int y, std::string new_txt, Color new_color, Font* new_font, bool _shadowed)
	: TComponent(0, y), font(new_font), color(new_color), shadowed(_shadowed), bg_offset(0), auto_set(true)
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

void TLabel::Init()
{
	if(auto_set && Window())
		SetX(Window()->GetWidth()/2 - font->GetWidth(caption)/2);
}

void TLabel::SetCaption (std::string new_txt)
{
	if(caption == new_txt)
		return;

	caption = new_txt;
	Reinit();
}

void TLabel::SetFontColor(Font* f, Color c)
{
	color = c;
	font = f;
	Reinit();
}

void TLabel::SetColor(Color new_color)
{
	if(color == new_color) return;
	color = new_color;
	Reinit();
}

void TLabel::SetFont(Font* f)
{
	font = f;
	Reinit();
}

void TLabel::Reinit()
{
	if(caption.empty())
	{
		surf.SetImage(0);
		background.SetImage(0);
		return;
	}

	surf = font->CreateSurface(caption,color);

	if(shadowed)
		background = font->CreateSurface(caption, white_color);

	SetHeight(surf.GetHeight());
	SetWidth(surf.GetWidth());
	if(auto_set && Window())
		SetX(Window()->GetWidth()/2 - font->GetWidth(caption)/2);
}

void TLabel::Draw(int m_x, int m_y)
{
	if(surf.IsNull() || caption.empty()) return; /* Possible (mais bizare). Par exemple un SpinEdit sans texte */

	assert(Window());

	SDL_Rect dst_rect;
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = surf.GetWidth();
	dst_rect.h = surf.GetHeight();

	if(shadowed && !background.IsNull())
	{
		SDL_Rect shad_rect = {x+bg_offset, y+bg_offset, background.GetWidth(), background.GetHeight()};

		Window()->Blit(background, &shad_rect);
	}
	Window()->Blit(surf, &dst_rect);
}
