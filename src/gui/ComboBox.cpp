/* src/gui/ComboBox.cpp - This component is an edit'form with a list
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

#include "ComboBox.h"
#include "tools/Color.h"
#include "Resources.h"
#include "tools/Font.h"

TComboBox::TComboBox(Font* f, int _x, int _y, uint _width)
	: TListBox(f, _x, _y, _width, f->GetHeight()), real_y(_y), opened(false), visible_len(0),
		  chaine(_x+5, _y, "", black_color, f), COMBOBOX_HEIGHT(f->GetHeight())
{
	gray_disable = true;
}

void TComboBox::Init()
{
  // Load images
  assert(font);
  visible_len = ((w) / font->GetWidth("A"));

  MyComponent(&m_open);
  MyComponent(&chaine);

  m_open.SetImage (new ECSprite(Resources::DownButton(), Window()));
  m_open.SetXY(x+w-12, y);

  SDL_Rect r_back = {0,0,w-13,COMBOBOX_HEIGHT};

  edit_bg.SetImage(SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w-13, COMBOBOX_HEIGHT,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
  edit_bg.FillRect(r_back, edit_bg.MapRGBA(255, 255, 255, 255*3/10));

  TListBox::Init();

  background.SetImage(0);
  nb_visible_items_max = uint(-1);
}

void TComboBox::SetBackGround(uint _h)
{
	SDL_Rect r_back = {0,0,w,_h};
	background.SetImage(SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, _h,
					32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
	background.FillRect(r_back, background.MapRGBA(255, 255, 255, 255*7/10));
}

void TComboBox::SetOpened(bool _o)
{
	if(m_items.empty())
		opened = false;
	else
		opened = _o;

	if(opened)
	{
		y = real_y + COMBOBOX_HEIGHT + 1;
		h = height_item * m_items.size();
		visible_height = h;
		m_open.SetImage (new ECSprite(Resources::UpButton(), Window()));
		SetBackGround(h);
	}
	else
	{
		y = real_y;
		h = COMBOBOX_HEIGHT;
		m_open.SetImage (new ECSprite(Resources::DownButton(), Window()));
	}
}

uint TComboBox::AddItem (bool selected,
		       const std::string &label,
		       const std::string &value, Color _color, bool enabled)
{
	uint j = TListBox::AddItem(selected, label, value, _color, enabled);
	if(opened)
	{
		h = height_item * m_items.size();
		visible_height = h;
		SetBackGround(h);
	}
	return j;
}

void TComboBox::ClearItems()
{
	TListBox::ClearItems();
	SetOpened(false);
	background.SetImage(0);
	chaine.SetCaption("");
}

void TComboBox::Select (uint index)
{
	if(index >= m_items.size() || !m_items[index].enabled) return;

	m_selection = index;

	std::string s = ReadLabel(m_selection);

	chaine.SetCaption((s.size() > visible_len) ? s.substr(0, visible_len) : s);
}

void TComboBox::Deselect (uint index)
{
	// On ne permet pas de deselectionner dans une combobox !
	return;
}

bool TComboBox::Clic (int mouse_x, int mouse_y)
{
	if(!enabled) return false;

	bool r = false;

	if(opened)
		r = TListBox::Clic(mouse_x,mouse_y);
	if(m_open.Test(mouse_x, mouse_y))
		SetOpened(!opened), r = true;
	if(opened && MouseIsOnWitchItem(mouse_x,mouse_y) != -1)
		SetOpened(false), r = true;

	if(!r && opened && !Test(mouse_x,mouse_y))
		SetOpened(false), r = true;

	return r;
}

void TComboBox::Draw (int mouse_x, int mouse_y)
{
	SDL_Rect r_back = {x,real_y,w-13,COMBOBOX_HEIGHT};
	Window()->Blit(edit_bg, &r_back);

	chaine.Draw(mouse_x, mouse_y);

	if(enabled)
		m_open.Draw(mouse_x,mouse_y);

	if(opened)
		TListBox::Draw(mouse_x,mouse_y);
}

void TComboBox::SetXY (int _x, int _y)
{
	x = _x;
	real_y = _y;
	chaine.SetXY(_x+5, real_y);
	m_open.SetXY(x+w-12, real_y);
	SetOpened(Opened());
}
