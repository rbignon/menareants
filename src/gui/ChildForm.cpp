/* src/gui/ChildForm.cpp - This is a form include in another form
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

#include "ChildForm.h"
#include "Main.h"

TChildForm::TChildForm(int _x, int _y, uint _w, uint _h)
	: TComponent(_x, _y, _w, _h)
{
	background = NULL;
}

void TChildForm::SetBackground(ECImage *image)
{
	background = image;
}

void TChildForm::Draw(int _x, int _y)
{
	if(background)
	{
		SDL_Rect r_back = {x,y,w,h};
		SDL_BlitSurface(background->Img,NULL,app.sdlwindow,&r_back);
	}

	bool first = true;
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
			if((*it)->Visible() && (*it)->Focused() == (first ? false : true)) // Affiche seulement à la fin les composants
				(*it)->Draw(_x, _y);                                            // selectionnés
		if(first) first = false;
		else break;
	}
}

void TChildForm::SetXY(int _x, int _y)
{
	for(std::vector<TComponent*>::iterator it =composants.begin(); it != composants.end(); ++it)
		(*it)->SetXY(((*it)->X() - x) + _x, ((*it)->Y() - y) + _y);
	x = _x;
	y = _y;
}
