/* src/gui/BouttonText.cpp - Boutton Text
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

#include "BouttonText.h"

TButtonText::TButtonText() : TButton(), font(0)
{
}

//-----------------------------------------------------------------------------

TButtonText::TButtonText (int _x, int _y, unsigned int _w, unsigned int _h, const std::string &text, Font *f)
  : TButton(_x, _y, _w, _h), label(_x, _y, text, white_color, f), font(f)
{

}
//-----------------------------------------------------------------------------

void TButtonText::Init()
{
	TButton::Init();
	MyComponent(&label);
}

void TButtonText::SetCaption(const std::string &text)
{
  label.SetCaption(text);
  label.SetXY(X()+Width()/2-label.Width()/2, Y()+Height()/2-label.Height()/2);
  SetWantRedraw();
}

void TButtonText::SetEnabled(bool b)
{
	TComponent::SetEnabled(b);
	label.SetColor(Enabled() ? white_color : gray_color);
	SetWantRedraw();
}

void TButtonText::SetXY(int _x, int _y)
{
	TButton::SetXY(_x, _y);
	label.SetXY(X()+Width()/2-label.Width()/2, Y()+Height()/2-label.Height()/2);
}

void TButtonText::SetImage (ECSprite *_image)
{
	TButton::SetImage(_image);

	label.SetXY(X()+Width()/2-label.Width()/2, Y()+Height()/2-label.Height()/2);
}

//-----------------------------------------------------------------------------

void TButtonText::Draw (const Point2i& mouse)
{
	if(image)
		DrawImage (mouse);

	label.Draw(mouse);
}

//-----------------------------------------------------------------------------

std::string TButtonText::Caption() const
{
  return label.Caption();
}
