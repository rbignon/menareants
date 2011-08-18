/* src/gui/ComboBox.cpp - This component is an edit'form with a list
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

#include "ComboBox.h"
#include "tools/Color.h"
#include "Resources.h"
#include "tools/Font.h"
#include <assert.h>

TComboBox::TComboBox(Font* f, int _x, int _y, uint _width)
	: TListBox(Rectanglei(_x, _y, _width, f->GetHeight())), real_y(_y), opened(false), visible_len(0),
		  chaine(_x+5, _y, "", white_color, f), COMBOBOX_HEIGHT(f->GetHeight()), font(f)
{

}

void TComboBox::Init()
{
  // Load images
  assert(font);
  visible_len = ((Width()) / font->GetWidth("A"));

  MyComponent(&m_open);
  MyComponent(&chaine);

  m_open.SetImage (new ECSprite(Resources::DownButton(), Window()));
  m_open.SetXY(X()+Width()-12, Y());

  // FIXME: What does 13 mean ?
  Rectanglei r(0, 0, Width()-13, COMBOBOX_HEIGHT);

  edit_bg.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, Width()-13, COMBOBOX_HEIGHT,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
  edit_bg.FillRect(r, edit_bg.MapColor(BoxColor));
  edit_bg.RectangleColor(r, white_color);

  box_color = BoxColor;

  TListBox::Init();
}

void TComboBox::SetOpened(bool _o)
{
	if(m_items.empty())
		opened = false;
	else
		opened = _o;

	if(opened)
	{
		position.y = real_y + COMBOBOX_HEIGHT + 1;
		size.y = CalculateHeight();
		m_open.SetImage (new ECSprite(Resources::UpButton(), Window()));
	}
	else
	{
		position.y = real_y;
		size.y = COMBOBOX_HEIGHT;
		m_open.SetImage (new ECSprite(Resources::DownButton(), Window()));
	}
}

int TComboBox::CalculateHeight()
{
	int h = 0;
	for(uint i=0; i < m_items.size(); i++)
		h += m_items.at(i)->Height();
	return h;
}

TListBoxItem* TComboBox::AddItem (bool selected,
                         const std::string &label,
                         const std::string &value,
                         const Color& color,
                         bool enabled,
                         Font& font,
                         const std::string& name)
{
	TListBoxItem* item = TListBox::AddItem(selected, label, value, color, enabled, font, name);
	item->SetGrayDisable();
	if(opened)
		size.y = CalculateHeight();

	return item;
}

void TComboBox::ClearItems()
{
	TListBox::ClearItems();
	SetOpened(false);
	chaine.SetCaption("");
}

void TComboBox::Select (uint index)
{
	if(index >= m_items.size()) return;

	selected_item = index;

	std::string s = m_items[index]->Label();

	chaine.SetCaption((s.size() > visible_len) ? s.substr(0, visible_len) : s);
}

void TComboBox::Deselect (uint index)
{
	// On ne permet pas de deselectionner dans une combobox !
	return;
}

bool TComboBox::Clic (const Point2i& mouse, int button)
{
	if(!Enabled()) return false;

	bool r = false;

	if(opened)
		r = TListBox::Clic(mouse, button);
	else if(Mouse(mouse)) // We can clic everywhere on the label and the button to open
		SetOpened(!opened), r = true;
	if(opened && MouseIsOnWhichItem(mouse) != -1)
		SetOpened(false), r = true;

	if(!r && opened && !Mouse(mouse))
		SetOpened(false), r = true;

	return r;
}

void TComboBox::Draw (const Point2i& mouse)
{
	SDL_Rect r_back = {X(),real_y,Width()-13,COMBOBOX_HEIGHT};
	Window()->Blit(edit_bg, &r_back);

	chaine.Draw(mouse);

	if(Enabled())
		m_open.Draw(mouse);

	if(opened)
		TListBox::Draw(mouse);
}

void TComboBox::SetXY (int _x, int _y)
{
	position.x = _x;
	real_y = _y;
	chaine.SetXY(_x+5, real_y);
	m_open.SetXY(X()+Width()-12, real_y);
	SetOpened(Opened());
}
