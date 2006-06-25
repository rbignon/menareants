/* src/gui/Component.cpp - Base of components
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
 * $Id$
 */

#include "Component.h"

/********************************************************************************************
 *                                 TList                                                    *
 ********************************************************************************************/

TList::TList(int _x, int _y)
	: TComponent(_x, _y)
{
	list.clear();
 dynamic_hint = true;
}

TList::~TList()
{
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
		delete (*it);
	list.clear();
}

void TList::AddLine(TComponent *c)
{
	list.push_back(c);
	c->SetParent(this);
	c->SetWindow(Window());
	c->SetXY(x, y+h);
	h += c->Height();
	if(c->Width() > w) w = c->Width();
	c->Init();
}

bool TList::RemoveLine(TComponent *c, bool use_delete)
{
	for (std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); )
	{
		if (*it == c)
		{
			h -= c->Height();
			if(use_delete)
				delete c;
			it = list.erase(it);
			Rebuild(); /* Reconstruction */
			return true;
		}
		else
			++it;
	}
	return false;
}

void TList::Rebuild()
{
	visible = false;
	h = 0;
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		(*it)->SetXY(x, y+h);
		h += (*it)->Height();
		if((*it)->Width() > w)
			w = (*it)->Width();
	}
	visible = true;
}

void TList::Draw(int souris_x, int souris_y)
{
	bool first = true, put_hint = false;
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
			if((*it)->Visible() && (*it)->Focused() == (first ? false : true))
			{ // Affiche seulement à la fin les composants selectionnés
				(*it)->Draw(souris_x, souris_y);
				if((*it)->OnMouseOn() && (*it)->Test(souris_x, souris_y))
					(*(*it)->OnMouseOn()) (*it, (*it)->OnMouseOnParam());
				if((*it)->Visible() && (*it)->Hint() && (*it)->Test(souris_x, souris_y))
				{
					SetHint((*it)->Hint());
					put_hint = true;
				}
			}
		if(first) first = false;
		else break;
	}
	if(!put_hint)
		SetHint("");
}

bool TList::Clic (int mouse_x, int mouse_y)
{
	bool click = false;

	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
		if((*it)->Visible() && !click && (*it)->Clic(mouse_x, mouse_y))
		{
			(*it)->SetFocus();
			if((*it)->OnClick())
				(*(*it)->OnClick()) (*it, (*it)->OnClickParam());
			if((*it)->OnClickPos())
				(*(*it)->OnClickPos()) (*it, mouse_x, mouse_y);
			click = true;
		}
		else if(!(*it)->ForceFocus())
			(*it)->DelFocus();

	return click;
}

void TList::DelFocus()
{
	TComponent::DelFocus();
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
		(*it)->DelFocus();
}

void TList::SetXY (int px, int py)
{
	TComponent::SetXY(px, py);
	Rebuild();
}

/********************************************************************************************
 *                                 TComponent                                               *
 ********************************************************************************************/

int TComponent::X() const { return x; }
int TComponent::Y() const { return y; }
unsigned int TComponent::Width() const { return w; }
unsigned int TComponent::Height() const { return h; }

void TComponent::SetFocus()
{
	focus = true;
}

void TComponent::DelFocus()
{
	focus = false;
}


bool TComponent::Test (int souris_x, int souris_y) const
{
  return (visible && ((x <= souris_x) && (souris_x < int(x+w))
	  && (y <= souris_y) && (souris_y < int(y+h))) && enabled);
}

void TComponent::SetX(int _x) { SetXY(_x, Y()); }
void TComponent::SetY(int _y) { SetXY(X(), _y); }
void TComponent::SetXY (int px, int py) { x = px; y = py; }

void TComponent::SetHeight (uint ph)
{
	if(h == ph) return;
	h = ph;
	Init();
}

void TComponent::SetWidth (uint pw) {
	if(w == pw) return;
	w = pw;
	Init();
}
