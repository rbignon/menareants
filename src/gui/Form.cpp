/* src/gui/Form.cpp - Form is base of a component's screen
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

#include "Form.h"
#include "Main.h"
#include <SDL.h>

TForm::TForm()
{
	background = NULL;
}

void TForm::SetBackground(ECImage *image)
{
	background = image;
}

void TForm::Update()
{
	Update(-1,-1,true);
}

void TForm::Update(int x, int y)
{
	Update(x,y,true);
}

void TForm::Update(bool flip)
{
	Update(-1,-1,flip);
}

void TForm::Update(int x, int y, bool flip)
{
	if(background)
		SDL_BlitSurface(background->Img,NULL,app.sdlwindow,NULL);

	if(x < 0 || y < 0)
		SDL_GetMouseState( &x, &y);

	size_t s = composants.size();
	for(uint i=0; i<s; i++)
		if(composants[i]->Visible())
			composants[i]->Draw(x, y);

	if(flip)
		SDL_Flip(app.sdlwindow);
}
