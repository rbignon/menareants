/* src/gui/CheckBox.cpp - ComboBox component.
 *
 * Copyright (C) 2005,2007 Romain Bignon  <Progs@headfucking.net>
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

#include "CheckBox.h"
#include "Resources.h"
#include "gui/Label.h"
#include "tools/Font.h"

TCheckBox::TCheckBox(Font* f, int _x, int _y, std::string txt, Color color)
	: TComponent(_x, _y), text(_x+20, _y, txt, color, f), image(0), checked(false)
{

}

TCheckBox::~TCheckBox()
{
	delete image;
}

void TCheckBox::Init()
{
	SetWidth(text.Width()+20);

	delete image;

	image = new ECSprite(Resources::CheckBox(), Window());
	SetHeight(image->GetHeight() > text.Height() ? image->GetHeight() : text.Height());
	image->set(X(), Y() + (text.Height() - image->GetHeight())/2);

	MyComponent(&text);
}

void TCheckBox::Draw(int _x, int _y)
{
	unsigned int frame = Test(_x,_y) ? (checked ? 3 : 1) : (checked ? 2 : 0);

	image->SetFrame(frame);
	image->draw();
	text.Draw(_x, _y);
}

void TCheckBox::SetXY (int _x, int _y)
{
	TComponent::SetXY(_x,_y);
	image->set(_x, _y + (text.Height() - image->GetHeight())/2);
	text.SetXY(_x+20, _y);
}

bool TCheckBox::Clic(int _x, int _y, int button)
{
	if(!TComponent::Clic(_x, _y, button))
		return false;

	checked = !checked;

	return true;
}
