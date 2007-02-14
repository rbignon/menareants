/* src/gui/Form.cpp - Form is base of a component's screen
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

#include "Form.h"
#include "Memo.h"
#include "MessageBox.h"
#include <SDL.h>

std::string TForm::Message;

TForm::TForm(ECImage* w)
	: want_quit(false), background(0), focus_order(true), Hint(0), mutex(0), max_fps(35), must_redraw(true)
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

	while((*func)() && !want_quit)
	{
		Actions();
		Update();
	}
}

void TForm::Run(uint a)
{
	while(!want_quit)
	{
		Actions(a);
		Update();
	}
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
		case SDL_KEYDOWN:
			if(!(a & ACTION_NOKEY))
			{
				OnKeyDown(event.key.keysym);
				for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
					(*it)->PressKey(event.key.keysym);
			}
			SetMustRedraw();
			break;
		case SDL_KEYUP:
			if(!(a & ACTION_NOKEY))
				OnKeyUp(event.key.keysym);
			SetMustRedraw();
			break;
		case SDL_MOUSEMOTION:
		{
			Point2i mouse(event.button.x,event.button.y);
			OnMouseMotion(mouse);
			if(Hint)
			{
				Hint->ClearItems();
				for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				{
					if((*it)->Visible() && (*it)->HaveHint() &&
					   ((*it)->DynamicHint() || (*it)->Mouse(mouse)))
					{
						Hint->AddItem((*it)->Hint());
						break;
					}
				}
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			OnClicUp(Point2i(event.button.x, event.button.y), event.button.button);
			SetMustRedraw();
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if(a & ACTION_NOMOUSE) break;
			bool click = false, stop = false;
			Point2i mouse(event.button.x, event.button.y);
			/* Va dans l'ordre inverse */
			for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				if((*it)->Visible() && (a & ACTION_NOCLIC) ? (*it)->Test(mouse, event.button.button)
				                                           : (*it)->Clic(mouse, event.button.button) && !click)
				{
					if(!(a & ACTION_NOFOCUS))
						(*it)->SetFocus();
					if((*it)->OnClick() && !(a & ACTION_NOCALL))
						(*(*it)->OnClick()) (*it, (*it)->OnClickParam());
					if((*it)->OnClickPos() && !(a & ACTION_NOCALL))
						(*(*it)->OnClickPos()) (*it, mouse);
					click = true;
				}
				else if(!(*it)->ForceFocus())
					(*it)->DelFocus();

			OnClic(mouse, event.button.button, stop);
			SetMustRedraw();
			break;
		}
		default:
			break;
	}
/*	if(mutex)
		SDL_UnlockMutex(mutex);*/
}

void TForm::Update(bool flip)
{
	BeforeDraw();

	if(!Message.empty())
	{
		TMessageBox(Message, BT_OK, this).Show();
		Message.clear();
	}

	unsigned int start, delay, sleep_fps;
	start = SDL_GetTicks();

	Draw();

	if(flip)
		Window()->Flip();

	delay = SDL_GetTicks()-start;
	if (delay < max_fps)
		sleep_fps = max_fps - delay;
	else
		sleep_fps = 0;
	if(sleep_fps >= SDL_TIMESLICE)
		SDL_Delay(sleep_fps);

	SetMustRedraw(false);

	AfterDraw();

	//SDL_Delay(15);
}

void TForm::Draw()
{
	int _x, _y;

	if(mutex)
		SDL_LockMutex(mutex);

	if(background && MustRedraw())
		Window()->Blit(background);

	SDL_GetMouseState( &_x, &_y);

	Point2i pos(_x, _y);

	bool first = focus_order ? true : false;
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
			// Affiche seulement à la fin les composants sélectionnés
			if((*it)->Visible() && (!focus_order || (*it)->Focused() == (first ? false : true)) &&
			   (MustRedraw() || (*it)->WantRedraw() || lastmpos != pos && (*it)->Mouse(pos) || (*it)->Mouse(lastmpos) && !(*it)->Mouse(pos)))
			{
				if(background && (*it)->RedrawBackground())
					Window()->Blit(background, **it, (*it)->GetPosition());
				if((*it)->OnMouseOn() && (*it)->Mouse(pos))
					(*(*it)->OnMouseOn()) (*it, (*it)->OnMouseOnParam());
				(*it)->Draw(pos);
				(*it)->SetWantRedraw(false);
			}
		if(first) first = false;
		else break;
	}

	lastmpos = pos;

	if(mutex)
		SDL_UnlockMutex(mutex);
}
