/* src/gui/Component.cpp - Base of components
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
	Clear();
}

void TList::Clear()
{
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
		delete (*it);
	list.clear();
	Rebuild();
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
				if((*it)->OnMouseOn() && (*it)->Mouse(souris_x, souris_y))
					(*(*it)->OnMouseOn()) (*it, (*it)->OnMouseOnParam());
				if((*it)->Visible() && !(*it)->Hint().empty() && (*it)->Mouse(souris_x, souris_y))
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

bool TList::Clic (int mouse_x, int mouse_y, int button)
{
	bool click = false;

	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); ++it)
		if((*it)->Visible() && !click && (*it)->Clic(mouse_x, mouse_y, button))
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
