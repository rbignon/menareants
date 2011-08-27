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
	: font(0), shadowed(0), bg_offset(0), auto_set(false), gray_disable(false)
{
}

TLabel::TLabel(const TLabel& label)
	: TComponent(label), font(label.font), color(label.color),
	  shadowed(label.shadowed), bg_offset(label.bg_offset), auto_set(label.auto_set), gray_disable(label.gray_disable)
{
	SetCaption(label.caption);
}

TLabel::TLabel(int x, int y, const std::string& new_txt, Color new_color, Font* new_font, bool _shadowed)
	: TComponent(x, y), font(new_font), color(new_color), shadowed(_shadowed), bg_offset(0), auto_set(false), gray_disable(false)
{
	assert(new_font!=NULL);
	SetCaption(new_txt);
}

TLabel::TLabel(int y, const std::string& new_txt, Color new_color, Font* new_font, bool _shadowed)
	: TComponent(0, y), font(new_font), color(new_color), shadowed(_shadowed), bg_offset(0), auto_set(true), gray_disable(false)
{
	assert(new_font!=NULL);
	SetCaption(new_txt);
}

void TLabel::Init()
{
	if (shadowed) {
		int width = font->GetWidth("x");
		bg_offset = (uint)(width>>3); // shadow offset = 0.125ex
		if (bg_offset < 1) {
			bg_offset = 1;
		}
	}

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

void TLabel::SetEnabled(bool _en)
{
	bool last = Enabled();
	TComponent::SetEnabled(_en);
	if(gray_disable && _en != last)
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

	Color txt_color = (!Enabled() && gray_disable) ? gray_color : color;

	surf = font->CreateSurface(caption, txt_color);

	if(shadowed)
		background = font->CreateSurface(caption, txt_color == white_color ? black_color : white_color);

	SetHeight(surf.GetHeight());
	SetWidth(surf.GetWidth());
	if(auto_set && Window())
		SetX(Window()->GetWidth()/2 - font->GetWidth(caption)/2);

	SetWantRedraw();
}

void TLabel::Draw(const Point2i& mouse)
{
	if(surf.IsNull() || caption.empty()) return; /* Possible (mais bizare). Par exemple un SpinEdit sans texte */

	assert(Window());

	if(shadowed && !background.IsNull()) {
		Window()->Blit(background, Point2i(X()-bg_offset, Y()-bg_offset));
		Window()->Blit(background, Point2i(X()-bg_offset, Y()+bg_offset));
		Window()->Blit(background, Point2i(X()+bg_offset, Y()+bg_offset));
		Window()->Blit(background, Point2i(X()+bg_offset, Y()-bg_offset));
	}

	Window()->Blit(surf, Point2i(X(), Y()));
}
