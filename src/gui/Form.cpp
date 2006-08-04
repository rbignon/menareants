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
#include "Memo.h"
#include <SDL.h>

TForm::TForm(ECImage* w)
	: background(0), focus_order(true), Hint(0), mutex(0), max_fps(50)
{
	SetWindow(w);
}

TForm::~TForm()
{
	Clear();
}

void TForm::Clear()
{
	for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
		delete *it;
	composants.clear();
}

void TForm::LockScreen() const
{
	if(mutex)
		SDL_LockMutex(mutex);
}

void TForm::UnlockScreen() const
{
	if(mutex)
		SDL_UnlockMutex(mutex);
}

void TForm::SetBackground(ECImage *image)
{
	background = image;
	if(background->GetWidth() != Window()->GetWidth() ||
	   background->GetHeight() != Window()->GetHeight())
		background->Zoom(double(Window()->GetWidth()) / (double)background->GetWidth(),
	                     double(Window()->GetHeight()) / (double)background->GetHeight(),
	                     true);
}

void TForm::Run(bool *(func)())
{
	if(!func) return;

	while((*func)())
	{
		Actions();
		Update();
	}
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
	/*if(mutex)
		SDL_LockMutex(mutex);*/
	switch(event.type)
	{
		case SDL_KEYUP:
			keys[event.key.keysym.sym] = false;
			break;
		case SDL_KEYDOWN:
			if(!(a & ACTION_NOKEY))
			{
				for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
					(*it)->PressKey(event.key.keysym);
			}
			keys[event.key.keysym.sym] = true;
			break;
		case SDL_MOUSEMOTION:
		{
			if(Hint)
			{
				Hint->ClearItems();
				bool put_hint = false;
				for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				{
					if(!put_hint && Hint && (*it)->Visible() && !(*it)->HaveHint() &&
					   ((*it)->DynamicHint() || (*it)->Mouse(event.button.x,event.button.y)))
					{
						Hint->AddItem((*it)->Hint());
						put_hint = true;
					}
				}
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if(a & ACTION_NOMOUSE) break;
			bool click = false;
			/* Va dans l'ordre inverse */
			for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				if((*it)->Visible() && !click && a & ACTION_NOCLIC ? (*it)->Test(event.button.x, event.button.y)
				                                                   : (*it)->Clic(event.button.x, event.button.y))
				{
					if(!(a & ACTION_NOFOCUS))
						(*it)->SetFocus();
					if((*it)->OnClick() && !(a & ACTION_NOCALL))
						(*(*it)->OnClick()) (*it, (*it)->OnClickParam());
					if((*it)->OnClickPos() && !(a & ACTION_NOCALL))
						(*(*it)->OnClickPos()) (*it, event.button.x, event.button.y);
					click = true;
				}
				else if(!(*it)->ForceFocus())
					(*it)->DelFocus();
			break;
		}
		default:
			break;
	}
/*	if(mutex)
		SDL_UnlockMutex(mutex);*/
}

void TForm::Update(int _x, int _y, bool flip)
{
	if(background)
		Window()->Blit(background);

	if(_x < 0 || _y < 0)
		SDL_GetMouseState( &_x, &_y);

	if(mutex)
		SDL_LockMutex(mutex);

	bool first = focus_order ? true : false;
	unsigned int start, delay, sleep_fps;
	start = SDL_GetTicks();
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
			// Affiche seulement � la fin les composants s�lectionn�s
			if((*it)->Visible() && (!focus_order || (*it)->Focused() == (first ? false : true)))
			{
				if((*it)->OnMouseOn() && (*it)->Mouse(_x, _y))
					(*(*it)->OnMouseOn()) (*it, (*it)->OnMouseOnParam());
				(*it)->Draw(_x, _y);
			}
		if(first) first = false;
		else break;
	}

	if(mutex)
		SDL_UnlockMutex(mutex);

	if(flip)
		Window()->Flip();

	delay = SDL_GetTicks()-start;
	if (delay < max_fps)
		sleep_fps = max_fps - delay;
	else
		sleep_fps = 0;
	if(sleep_fps >= SDL_TIMESLICE)
		SDL_Delay(sleep_fps);

	//SDL_Delay(15);
}
