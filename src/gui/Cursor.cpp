/* src/gui/Cursor.cpp - Mouse
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

#include "Cursor.h"
#include "Resources.h"
#include "gui/ShowMap.h"
#include <SDL_mouse.h>

TCursor Cursor;

TCursor::TCursor()
	: initialized(false), map(0), pointer(TOPLEFT_POINTERS)
{
	SetAlwaysRedraw();
}

TCursor::~TCursor()
{
	SDL_ShowCursor(true);
}

void TCursor::Init()
{
	if(initialized) return;

	SDL_ShowCursor(false);
	SetCursorImage(Standard, Resources::PointerStandard());
	SetCursorImage(Select, Resources::PointerSelect());
	SetCursorImage(Attaq, Resources::PointerAttaq());
	SetCursorImage(MaintainedAttaq, Resources::PointerMaintainedAttaq());
	SetCursorImage(CantAttaq, Resources::PointerCantAttaq());
	SetCursorImage(Invest, Resources::PointerInvest());
	SetCursorImage(Left, Resources::PointerLeft());
	SetCursorImage(Radar, Resources::PointerRadar());
	SetCursorImage(AddBP, Resources::PointerAddBP());
	SetCursorImage(RemBP, Resources::PointerRemBP());

	SetCursor(Standard);

	SDL_WarpMouse(Window()->GetWidth()/2, Window()->GetHeight()/2);
	initialized = true;
}

void TCursor::Draw(const Point2i& pos)
{
	SetXY(pos.X(), pos.Y());

	ECImage* img = cursors[pointer];

	if(!img) return;

	Point2i p = pos;

	if(pointer > MIDDLE_POINTERS)
	{
		p.x = pos.X() - img->GetWidth() /2;
		p.y = pos.Y() - img->GetHeight()/2;
	}

	Window()->Blit(img, p);

	if(Map())
		Map()->ToRedraw(Rectanglei(p.X(), p.Y(), img->GetWidth(), img->GetHeight()));
}

void TCursor::SetCursor(cursors_t p)
{
	if (pointer == p || p == TOPLEFT_POINTERS || p == MIDDLE_POINTERS) return;

	pointer = p;
	ECImage* img = cursors[pointer];
	SetHeight(img->GetHeight());
	SetWidth(img->GetWidth());
}

void TCursor::SetCursorImage(cursors_t i, ECImage* b)
{
	if(i == TOPLEFT_POINTERS || i == MIDDLE_POINTERS) return;

	cursors[i] = b;
}
