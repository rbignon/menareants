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
#include "gui/Cursor.h"
#include "MessageBox.h"
#include <SDL.h>

std::string TForm::Message;

TForm::TForm(ECImage* w)
	: want_quit(false), background(0), focus_order(true),
	  Hint(Font::GetInstance(Font::Small), 0, 0, 200, 800, 0, false),
	  mutex(0), max_fps(35), must_redraw(true), last_move_time(0)
{
	SetWindow(w);
	MyComponent(&Hint);
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

void TForm::RemoveComponent(TComponent* comp)
{
	LockScreen();
	for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end();)
		if(*it == comp)
		{
			delete *it;
			it = composants.erase(it);
		}
		else
			++it;

	UnlockScreen();
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

			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			if(a & ACTION_NOMOUSE) break;
			Point2i mouse(event.button.x, event.button.y);

			for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				if((*it)->Visible() && !(a & ACTION_NOCLIC))
					(*it)->ClicUp(mouse, event.button.button);

			OnClicUp(mouse, event.button.button);
			SetMustRedraw();
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if(a & ACTION_NOMOUSE) break;
			bool stop = false;
			TComponent* clicked = NULL;
			Point2i mouse(event.button.x, event.button.y);

			/* firstly loop on focused widgets, then on others, in reverse orders. */
			bool first = focus_order;
			while (1)
			{
				for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
				{
					if((*it)->Visible() && (!focus_order || (*it)->Focused() == first) && !clicked &&
					   ((a & ACTION_NOCLIC) ? (*it)->Test(mouse, event.button.button)
					                        : (*it)->Clic(mouse, event.button.button)))
					{
						if(!(a & ACTION_NOFOCUS))
							(*it)->SetFocus();
						if((*it)->OnClick() && !(a & ACTION_NOCALL))
							(*(*it)->OnClick()) (*it, (*it)->OnClickParam());
						if((*it)->OnClickPos() && !(a & ACTION_NOCALL))
							(*(*it)->OnClickPos()) (*it, mouse);
						clicked = *it;
					}
					else if(!first && clicked != *it && !(*it)->ForceFocus())
						(*it)->DelFocus();
				}
				if(first) first = false;
				else break;
			}

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
	if(!Message.empty())
	{
		TMessageBox(Message, BT_OK, this).Show();
		Message.clear();
	}

	unsigned int start, delay, sleep_fps;
	start = SDL_GetTicks();

	int _x, _y;
	SDL_GetMouseState( &_x, &_y);
	Point2i pos(_x,_y);

	if(pos != lastmpos)
	{
		OnMouseMotion(pos);
		last_move_time = SDL_GetTicks();
		if(Hint.Visible())
		{
			Hint.Hide();
			SetMustRedraw();
		}

		Hint.ClearItems();
		for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
		{
			if((*it)->Visible() && (*it)->IsHint(pos))
			{
				Hint.AddItem((*it)->Hint());
				break;
			}
		}
	}


	if(!Hint.Visible() && !Hint.Empty() && last_move_time <= SDL_GetTicks()-500)
		Hint.Show();

	Draw();

	Cursor.Draw(pos);

	lastmpos = pos;

	if(Hint.Visible())
	{
		Hint.SetXY((_x + Hint.Width())  > Width()  ? (_x - Hint.Width())  : (_x),
		           (_y + Cursor.Height() + Hint.RealHeight())  > Height()  ? (_y  - Hint.RealHeight())  : (_y + Cursor.Height()));
		Hint.Draw(Point2i(_x,_y));
	}

	if(flip)
		Window()->Flip();

	delay = SDL_GetTicks()-start;
	if (delay < max_fps)
		sleep_fps = max_fps - delay;
	else
		sleep_fps = 0;
	if(sleep_fps >= SDL_TIMESLICE)
		SDL_Delay(sleep_fps);

	AfterDraw();

	//SDL_Delay(15);
}

void TForm::Draw()
{
	int _x, _y;

	BeforeDraw();

	if(mutex)
		SDL_LockMutex(mutex);

	LockedBeforeDraw();

	if(want_quit)
	{
		SDL_UnlockMutex(mutex);
		return;
	}

	if(background)
	{
		if (MustRedraw())
			Window()->Blit(background);
		else
		{
			Window()->Blit(background, Cursor, Cursor.GetPosition());
			if (Hint.Visible())
				Window()->Blit(background, Hint, Hint.GetPosition());
		}
	}

	SDL_GetMouseState( &_x, &_y);

	Point2i pos(_x, _y);

	bool first = focus_order ? true : false;
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
			// Affiche seulement à la fin les composants sélectionnés
			if((*it)->Visible() && (!focus_order || (*it)->Focused() == (first ? false : true)) &&
			   (MustRedraw() || (*it)->AlwaysRedraw() || (*it)->WantRedraw() || (*it)->Intersect(Cursor) ||
			    ((*it)->Mouse(lastmpos) && !(*it)->Mouse(pos)) ||
			    (Hint.Visible() && (*it)->Intersect(Hint.GetRectangle()))))
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

	SetMustRedraw(false);

	if(mutex)
		SDL_UnlockMutex(mutex);
}
