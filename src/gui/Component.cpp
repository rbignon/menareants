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

TList::TList(uint _x, uint _y)
	: TComponent(_x, _y)
{
	list.clear(); /* On ne sait jamais */
}

TList::~TList()
{
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); it++)
		delete (*it);
	list.clear();
}

void TList::AddLine(TComponent *c)
{
	list.push_back(c);
	c->SetXY(x, y+h);
	h += c->GetHeight();
	if(c->GetWidth() > w) w = c->GetWidth();
	c->Init();
}

bool TList::RemoveLine(TComponent *c)
{
	for (std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); )
	{
		if (*it == c)
		{
			h -= c->GetHeight();
			it = list.erase(it);
			delete (*it);
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
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); it++)
	{
		(*it)->SetXY(x, y+h);
		h += (*it)->GetHeight();
		if((*it)->GetWidth() > w)
			w = (*it)->GetWidth();
	}
	visible = true;
}

void TList::Draw(uint souris_x, uint souris_y)
{
	for(std::vector<TComponent*>::iterator it = list.begin(); it != list.end(); it++)
		(*it)->Draw(souris_x, souris_y);
}

void TList::SetXY (uint px, uint py)
{
	TComponent::SetXY(px, py);
	Rebuild();
}

/********************************************************************************************
 *                                 TComponent                                               *
 ********************************************************************************************/

unsigned int TComponent::GetX() const { return x; }
unsigned int TComponent::GetY() const { return y; }
unsigned int TComponent::GetWidth() const { return w; }
unsigned int TComponent::GetHeight() const { return h; }

void TComponent::SetXY (uint px, uint py) { x = px; y = py; Init(); }

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
