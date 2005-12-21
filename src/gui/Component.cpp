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

unsigned int TComponent::GetX() const { return x; }
unsigned int TComponent::GetY() const { return y; }
unsigned int TComponent::GetWidth() const { return w; }
unsigned int TComponent::GetHeight() const { return h; }

void TComponent::SetXY (uint px, uint py) { x = px; y = py; }

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
