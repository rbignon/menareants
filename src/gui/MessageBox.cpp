/* src/gui/MessageBox.cpp - Show a window with a message.
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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

#include "MessageBox.h"
#include "Defines.h"
#include "Debug.h"
#include "tools/Video.h"
#include "Resources.h"
#include "tools/Font.h"
#include "i18n.h"
#include "gui/Cursor.h"

#include "tools/Maths.h"
#include <algorithm>

static struct ButtonList_t
{
	uint flag;
	char* label;
	uint w;
	uint h;
} ButtonList[] = {
	{ BT_OK,        gettext_noop("OK"),       100,    30  },
	{ BT_YES,       gettext_noop("Yes"),      100,    30  },
	{ BT_NO,        gettext_noop("No"),       100,    30  },
	{ BT_CANCEL,    gettext_noop("Cancel"),   100,    30  }
};

TMessageBox::TMessageBox(std::string _s, uint _b, TForm* form, bool transparence)
	: TObject(Video::GetInstance()->Window()), x(-1), y(-1), b(_b)
{
	edit = 0;
	Form = form;
	realbg = 0;
	Init(_s, transparence);
}

TMessageBox::TMessageBox(int _x, int _y, std::string _s, uint _b, TForm* form, bool transparence)
	: TObject(Video::GetInstance()->Window()), x(_x), y(_y), b(_b)
{
	edit = 0;
	Form = form;
	realbg = 0;
	Init(_s, transparence);
}

TMessageBox::~TMessageBox()
{
	for(std::vector<TButtonText*>::iterator it = boutons.begin(); it != boutons.end(); ++it)
		delete *it;

	delete edit;
}

uint TMessageBox::Show()
{
	SDL_Event event;
	try
	{
		do
		{
			while( SDL_PollEvent( &event) )
			{
				switch(event.type)
				{
					case SDL_KEYDOWN:
						switch (event.key.keysym.sym)
						{
							case SDLK_RETURN:
								if((!edit || edit->Focused()) && !boutons.empty())
									return boutons[0]->Tag;
								break;
							default: break;
						}
						if(edit)
							edit->PressKey(event.key.keysym);
						break;

					case SDL_MOUSEBUTTONDOWN:
					{
						Point2i mouse(event.button.x, event.button.y);
						if(edit)
							edit->Clic(mouse, event.button.button);
						for(uint i=0; i<boutons.size();i++)
							if(boutons[i]->Clic(mouse, event.button.button))
								return boutons[i]->Tag;
						break;
					}
					default:
						break;
				}
			}
			Draw();
		} while(1);
	}
	catch(...)
	{
		Debug(W_SEND|W_ERR, "Erreur dans un TMessageBox");
		throw;
	}
	return 0;
}

void TMessageBox::Draw()
{
	int x,y;
	unsigned int start, delay, sleep_fps;
	start = SDL_GetTicks();

	SDL_GetMouseState( &x, &y);

	if(realbg)
		Window()->Blit(realbg);
	if(Form)
	{
		Form->SetMustRedraw();
		Form->Draw();
	}
	Draw(x,y);
	Window()->Flip();

	delay = SDL_GetTicks()-start;
	if (delay < 35)
		sleep_fps = 35 - delay;
	else
		sleep_fps = 0;
	if(sleep_fps >= SDL_TIMESLICE)
		SDL_Delay(sleep_fps);
}

void TMessageBox::Draw(uint mouse_x, uint mouse_y)
{
	SDL_Rect r_back = {x,y,w,h};
	Window()->Blit(background, &r_back);

	uint vert = y;

	Point2i mouse(mouse_x, mouse_y);

	for (std::vector<TLabel>::iterator it = message.begin(); it != message.end(); ++it, vert += height_string)
	{
		it->SetXY(x+25, vert);
		it->Draw(mouse);
	}

	if(edit)
	{
		vert += 10;
		edit->Draw(mouse);
		vert += edit->Height();
	}

	vert += 20;

	for(std::vector<TButtonText*>::iterator it = boutons.begin(); it != boutons.end(); ++it)
		(*it)->Draw(mouse);

	if(Form)
		Form->SetMustRedraw();

	Cursor.Draw(mouse);

	return;
}

void TMessageBox::SetText(std::string __s)
{
	height_string = Font::GetInstance(Font::Normal)->GetHeight();
	std::string::const_iterator _s = __s.begin();

	/* On parse le message pour le découper en différentes lignes */
	char s[MSGBOX_MAXWIDTH + 20 + 2];
	for(uint i=0;;)
	{
		if(*_s == '\n' || ((i > MSGBOX_MAXWIDTH) && *_s == ' ') || (i > (MSGBOX_MAXWIDTH+20)) || _s == __s.end() || !(*_s))
		{ /* Retour à la ligne. Si ça dépasse le MSGBOX_MAXWIDTH lettres, on laisse une chance
		   * à un caractère ' ' ou '\n' de s'interposer pour couper proprement. A partir de
		   * MSGBOX_MAXWIDTH + 20 on coupe net.
		   */
		    s[i] = '\0';
			message.push_back(TLabel(x,y,s,black_color,Font::GetInstance(Font::Normal)));
			MyComponent(&(*(message.end()-1)));
			i=0;

			h += height_string;
			uint yw = Font::GetInstance(Font::Normal)->GetWidth(s) + 50;
			if(w < yw) w = yw;

			if(*_s == '\n')
				++_s; /* Seulement une fois, pour retourner à la ligne si il y en a un autre */
			while(*_s == ' ') ++_s;
			if(_s == __s.end() || !*_s) break;
		}
		else
			s[i++] = *_s++;
	}
}

void TMessageBox::SetEdit()
{
	if(edit) delete edit;
	h += 10;
	edit = new TEdit(Font::GetInstance(Font::Normal), x+15,y+h,w-15-20);
	MyComponent(edit);
	edit->SetFocus();
	h += edit->Height();
}

void TMessageBox::Init(std::string s, bool transparence)
{
	h = 0;
	w = 0;
	SetText(s);
	if(b & HAVE_EDIT)
		SetEdit();
	SetButtons();

	SDL_Rect r_back = {0,0,w,h};

	if(transparence)
	{
		background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, w, h,
		                                          32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
		background.FillRect(r_back, background.MapRGBA(172, 183, 255, 255*6/10));
	}
	else
	{
		background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE, w, h,
		                                          32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
		background.FillRect(r_back, background.MapRGBA(172, 183, 255, 255));
	}

	if(x == -1 && y == -1)
	{
		x = Video::GetInstance()->Width()/2 - w / 2;
		y = Video::GetInstance()->Height()/2 - h / 2;
		for(std::vector<TButtonText*>::iterator it = boutons.begin(); it != boutons.end(); ++it)
			(*it)->SetXY((*it)->X() + x + 1, (*it)->Y() + y + 1);
		if(edit)
			edit->SetXY(edit->X() + x + 1, edit->Y() + y + 1);
	}
}

void TMessageBox::SetButtons()
{
	if(!boutons.empty())
	{
		for(std::vector<TButtonText*>::iterator it = boutons.begin(); it != boutons.end(); ++it)
			delete *it;
		boutons.clear();
	}
	h += 20;

	/* Préparation des boutons */
	uint tmpw = x + 15, tmph = 0;
	for(uint i=0;i<ASIZE(ButtonList);i++)
		if(b & ButtonList[i].flag)
		{
			TButtonText *bt = new TButtonText(tmpw, y+h, ButtonList[i].w, ButtonList[i].h,
			                                  gettext(ButtonList[i].label), Font::GetInstance(Font::Normal));
			MyComponent(bt);
			bt->Tag = ButtonList[i].flag;
			bt->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));
			boutons.push_back(bt);
			tmpw += ButtonList[i].w + 1;
			if(tmph < ButtonList[i].h) tmph = ButtonList[i].h;
		}

	if(w < (tmpw-x + 20)) w = tmpw - x + 20;
	h += tmph + 5;
}
