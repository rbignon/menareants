/* src/gui/ListBox.cpp - A component to show a list of items
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
/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Listbox
 *****************************************************************************/

#include "ListBox.h"
#include <algorithm>
#include <SDL_gfxPrimitives.h>
#include "Resources.h"

#define SCROLLBAR

TListBoxItem::TListBoxItem(const std::string& _name, const std::string& _label, Font* _font, const std::string& _value, const Color& color)
	: TLabel(0,0, _label, color, _font), value(_value), name(_name)
{

}

const std::string& TListBoxItem::Label() const
{
	return Caption();
}

const std::string& TListBoxItem::Value() const
{
	return value;
}

const std::string& TListBoxItem::Name() const
{
	return name;
}

struct CompareItems
{
     bool operator()(const TListBoxItem* a, const TListBoxItem* b)
     {
       return a->Label() < b->Label();
     }
};

TListBox::TListBox (const Rectanglei &rect, bool always_one_selected_b, Font* _font)
	: TComponent(rect), no_item_hint(false), scrolling_delta(-1), box_color(BoxColor), on_change(0), gray_disable(true), font(_font)
{
	first_visible_item = 0;
	selected_item = -1;
	always_one_selected = always_one_selected_b;
	nb_visible_items_max = Height() / font->GetHeight();
}

void TListBox::Init()
{
	MyComponent(&m_up);
	MyComponent(&m_down);
	// Load images
	m_up.SetImage (new ECSprite(Resources::UpButton(), Window()));
	m_down.SetImage (new ECSprite(Resources::DownButton(), Window()));
	SetXY(X(), Y());
}

void TListBox::SetXY (int _x, int _y)
{
	TComponent::SetXY(_x, _y);
	m_up.SetXY(X() + Width() - 12, Y() + 2);
	m_down.SetXY(X() + Width() - 12, Y() + Height() - 12);
}

TListBox::~TListBox()
{
	ClearItems();
}

int TListBox::MouseIsOnWhichItem(const Point2i &mousePosition) const
{
	if( !Contains(mousePosition) )
		return -1;

	for (uint i=first_visible_item; i < m_items.size(); i++)
		if (m_items[i]->Y() <= mousePosition.y &&
		    m_items[i]->Y() + m_items[i]->Height() >= mousePosition.y &&
		    mousePosition.x < X()+Width()-12)
			return i;

	return -1;
}

void TListBox::ClicUp (const Point2i& pos, int button)
{
	scrolling_delta = -1;
}

bool TListBox::Clic(const Point2i &mousePosition, int button)
{
	if(!Enabled() || !Mouse(mousePosition)) return false;

	if (m_items.empty())
		return false;

	if(button == SDL_BUTTON_LEFT && ScrollBarPos().Contains(mousePosition))
		scrolling_delta = mousePosition.y - ScrollBarPos().Y();

	// buttons for listbox with more items than visible (first or last item not visible)
	if(m_down.Visible())
	//if(first_visible_item > 0 || first_visible_item+1 < m_items.size()/*m_items.back()->Y() + m_items.back()->Height() > Y() + Height()*/)
	{
		if(button == SDL_BUTTON_WHEELDOWN || (button == SDL_BUTTON_LEFT && m_down.Mouse(mousePosition)))
		{
			// bottom button
			if( first_visible_item < (m_items.size() - nb_visible_items_max) /*m_items.back()->Y() + m_items.back()->Height() > Y() + Height()*/ )
				first_visible_item++ ;

			return true;
		}
		else if(button == SDL_BUTTON_WHEELUP || (button == SDL_BUTTON_LEFT && m_up.Mouse(mousePosition)))
		{
			// top button
			if( first_visible_item > 0 )
				first_visible_item-- ;

			return true;
		}
	}

	if( button == SDL_BUTTON_LEFT )
	{
		int item = MouseIsOnWhichItem(mousePosition);

		if( item == -1 )
			return false;

		if(item != selected_item && m_items[item]->Enabled())
			Select(item);
		else if(!always_one_selected)
			Deselect ();
		else
			return false;

		if(on_change)
			(*on_change) (this);
		return true;
	}
	return false;
}

void TListBox::Draw(const Point2i &mousePosition)
{
	int item = MouseIsOnWhichItem(mousePosition);
	Rectanglei rect (X(), Y(), Width()-12, Height());
	Rectanglei scrollbar = ScrollBarPos();

	// Draw border and bg color
	Window()->BoxColor(rect, box_color);

	if(scrolling_delta >= 0)
	{
		int y = mousePosition.y - scrolling_delta;
		if(y + scrollbar.Height() > Y() + Height() - 12)
			first_visible_item = m_items.size() - nb_visible_items_max;
		else if(y < Y() + 12)
			first_visible_item = 0;
		else
			first_visible_item = (y - Y() - 10) * m_items.size() / (Height()-20);
	}

	// Draw items
	Point2i pos = GetPosition() + Point2i(5, 0);
	uint local_max_visible_items = m_items.size();
	bool draw_it = true;

	if(!NoItemHint())
		SetHint("");

	for(uint i=first_visible_item; i < m_items.size(); i++)
	{
		Rectanglei rect(X(), pos.Y(), Width()-12, m_items.at(i)->Height());

		// no more place to add item
		if (draw_it && rect.Y() + rect.Height() > Y() + Height())
		{
			local_max_visible_items = i - first_visible_item;
			draw_it = false;
		}

		// item is selected or mouse-overed
		if (draw_it)
		{
			if(Enabled() && i == uint(item))
			{
				if(m_items.at(i)->Enabled())
					Window()->BoxColor(rect, BoxColorMoused);
				if(!NoItemHint())
					SetHint(m_items.at(i)->Hint());
			}
			else if(int(i) == selected_item)
				Window()->BoxColor(rect, BoxColorSelected);
		}

		// Really draw items
		Rectanglei rect2(pos.x, pos.y, Width()-12, m_items.at(i)->Height());

		m_items.at(i)->SetXY(pos.x, pos.y);
		if (draw_it)
			m_items.at(i)->Draw(mousePosition);

		pos += Point2i(0, m_items.at(i)->Height());
	}

	// buttons for listbox with more items than visible
	if (m_items.size() > local_max_visible_items || first_visible_item > 0)
	{
		m_down.Show();
		m_up.Draw(mousePosition);
		m_down.Draw(mousePosition);
		Window()->BoxColor(scrollbar, (scrolling_delta >= 0 || scrollbar.Contains(mousePosition)) ? white_color : gray_color);
		Window()->RectangleColor(scrollbar, /*(scrolling_delta >= 0 || scrollbar.Contains(mousePosition)) ? gray_color : */white_color);
	}
	else
		m_down.Hide();
	Window()->RectangleColor(rect, white_color);
}

Rectanglei TListBox::ScrollBarPos() const
{
	if(m_items.empty())
		return Rectanglei(0,0,0,0);

	uint tmp_y, tmp_h;
	tmp_y = Y()+10+ first_visible_item* (Height()-20) / m_items.size();
	tmp_h = nb_visible_items_max * (Height()-20) / m_items.size();
	if (tmp_h < 5) tmp_h =5;

	return Rectanglei(X()+Width()-11, tmp_y, 9, tmp_h);
}

TListBoxItem* TListBox::AddItem (bool selected,
                        const std::string &label,
                        const std::string &value,
                        const Color& color,
                        bool enabled,
                        const std::string& name)
{
	uint pos = m_items.size();

	// Push item
	TListBoxItem * item = new TListBoxItem(name, label, font, value, color);
	MyComponent(item);
	item->SetEnabled(enabled);
	item->SetGrayDisable();
	m_items.push_back (item);

	// Select it if selected
	if( selected )
		Select (pos);

	SetWantRedraw();

	return item;
}

void TListBox::Sort()
{
  std::sort( m_items.begin(), m_items.end(), CompareItems() );
  SetWantRedraw();
}

void TListBox::ScrollTo(TListBoxItem* item)
{
	uint i = 0;
	for(std::vector<TListBoxItem*>::const_iterator it = m_items.begin(); it != m_items.end() && *it != item; ++it)
		i++;
	ScrollTo(i);
}

void TListBox::ScrollTo(uint id)
{
	if(id == first_visible_item || id >= m_items.size())
		return;

	if (nb_visible_items_max > m_items.size())
		first_visible_item = 0;
	else if (id > (m_items.size() - nb_visible_items_max/2))
		first_visible_item = m_items.size() - nb_visible_items_max;
	else if (id < nb_visible_items_max/2)
		first_visible_item = 0;
	else
		first_visible_item = id - nb_visible_items_max/2;

	SetWantRedraw();
}

void TListBox::ClearItems()
{
	for(std::vector<TListBoxItem*>::iterator it = m_items.begin(); it != m_items.end(); ++it)
		delete *it;

	m_items.clear();
	selected_item = -1;
	first_visible_item = 0;
	SetWantRedraw();
}

void TListBox::RemoveSelected()
{
	if( selected_item != -1 )
	{
		m_items.erase( m_items.begin() + selected_item );
		selected_item =- 1;
		SetWantRedraw();
	}
}

void TListBox::Select (uint index)
{
	assert(index < m_items.size());
	selected_item = index;
	SetWantRedraw();
}

void TListBox::Select(const std::string& val)
{
	uint index = 0;
	for(std::vector<TListBoxItem*>::iterator it=m_items.begin();
	    it != m_items.end();
	    it++,index++)
	{
		if((*it)->Label() == val)
		{
			Select(index);
			return;
		}
	}
}

void TListBox::Deselect ()
{
	assert (always_one_selected == false);
	selected_item = -1;
	SetWantRedraw();
}

int TListBox::Selected() const
{
	return selected_item;
}

TListBoxItem* TListBox::Item(uint i) const
{
	assert(i < m_items.size());
	return m_items.at(i);
}

TListBoxItem* TListBox::SelectedItem () const
{
	if(selected_item < 0)
		return 0;
	return m_items.at(selected_item);
}

const std::string& TListBox::ReadLabel () const
{
	assert (selected_item >= 0);
	return m_items.at(selected_item)->Label();
}

const std::string& TListBox::ReadValue () const
{
	assert (selected_item >= 0);
	return m_items.at(selected_item)->Value();
}

const std::string& TListBox::ReadValue (int index) const
{
	assert (index >= 0 && index < (int)m_items.size());
	return m_items.at(index)->Value();
}

const std::string& TListBox::ReadName (int index) const
{
	assert (index >= 0 && index < (int)m_items.size());
	return m_items.at(index)->Name();
}

void TListBox::SetEnabled(bool _en)
{
	TComponent::SetEnabled(_en);
	for(std::vector<TListBoxItem*>::iterator it = m_items.begin(); it != m_items.end(); ++it)
		(*it)->SetEnabled((Enabled() || !gray_disable));
}

uint TListBox::Size() const
{
	return m_items.size();
}
