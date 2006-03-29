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

void TForm::Actions(uint a)
{
	SDL_Event event;
	while( SDL_PollEvent( &event) )
	{
		Actions(event, a);
	}
}

void TForm::Actions(SDL_Event event, uint a)
{
	switch(event.type)
	{
		case SDL_KEYUP:
			if(!(a & ACTION_NOKEY))
			{
				for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
					(*it)->PressKey(event.key.keysym);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		{
			if(a & ACTION_NOMOUSE) break;
			/* Va dans l'ordre inverse */
			for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				if(a & ACTION_NOCLIC ? (*it)->Test(event.button.x, event.button.y)
				                     : (*it)->Clic(event.button.x, event.button.y))
				{
					if(!(a & ACTION_NOFOCUS))
						(*it)->SetFocus();
					if((*it)->ClickedFunc() && !(a & ACTION_NOCALL))
						(*(*it)->ClickedFunc()) (this, (*it)->ClickedFuncParam());
					break;
				}
				else
					(*it)->DelFocus();
			break;
		}
		default:
			break;
	}
}

void TForm::Update(int x, int y, bool flip)
{
	if(background)
		SDL_BlitSurface(background->Img,NULL,app.sdlwindow,NULL);

	if(x < 0 || y < 0)
		SDL_GetMouseState( &x, &y);

	bool first = true;
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
			if((*it)->Visible() && (*it)->Focused() == (first ? false : true)) // Affiche seulement � la fin les composants
				(*it)->Draw(x, y);                                             // selectionn�s
		if(first) first = false;
		else break;
	}

	if(flip)
		SDL_Flip(app.sdlwindow);
}
