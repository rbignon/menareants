/* src/gui/MessageBox.cpp - Show a window with a message.
 *
 * Copyright (C) 2005-2006 Romain Bignon  <Progs@headfucking.net>
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
#include "Main.h"
#include "Debug.h"

#include "tools/Maths.h"
#include <algorithm>

static struct ButtonList_t
{
	uint flag;
	char* label;
	uint w;
	uint h;
} ButtonList[] = {
	{ BT_OK,		"OK",		150,	50	},
	{ BT_YES,		"Oui",		150,	50	},
	{ BT_NO,		"Non",		150,	50	},
	{ BT_CANCEL,	"Annuler",	150,	50	}
};

uint TMessageBox::Show()
{
	SDL_Event event;
	try
	{
		do
		{
			int x=0, y=0;
			while( SDL_PollEvent( &event) )
			{
				switch(event.type)
				{
					case SDL_KEYUP:
						if(Edit)
						{
							switch (event.key.keysym.sym)
							{
								case SDLK_RETURN:
									if(Edit->Focused() && !boutons.empty())
										return boutons[0]->Tag;
									break;
								default: break;
							}
							Edit->PressKey(event.key.keysym);
						}
						break;

					case SDL_MOUSEBUTTONDOWN:
						if(Edit)
							Edit->Clic(event.button.x, event.button.y);
						for(uint i=0; i<boutons.size();i++)
							if(boutons[i]->Test(event.button.x, event.button.y))
								return boutons[i]->Tag;
						break;
					default:
						break;
				}
			}
			SDL_GetMouseState( &x, &y);
			if(realbg)
				SDL_BlitSurface(realbg->Img, NULL, app.sdlwindow, NULL);
			if(Form)
				Form->Update(x,y,false);
			Draw(x,y);
			SDL_Flip(app.sdlwindow);
		} while(1);
	}
	catch(...)
	{
		Debug(W_SEND|W_ERR, "Erreur dans un TMessageBox");
		throw;
	}
	return 0;
}

void TMessageBox::Draw(uint mouse_x, uint mouse_y)
{
	SDL_Rect r_back = {x,y,w,h};
	SDL_BlitSurface( background, NULL, app.sdlwindow, &r_back);

	uint vert = y;

	for (uint i=0; i < message.size(); i++, vert += height_string)
		app.Font()->normal.WriteLeft(x+25, vert, message[i], black_color);

	if(Edit)
	{
		vert += 10;
		Edit->Draw(mouse_x, mouse_y);
		vert += EDIT_HEIGHT;
	}

	vert += 20;

	for(std::vector<TButtonText*>::iterator it = boutons.begin(); it != boutons.end(); ++it)
		(*it)->Draw(mouse_x, mouse_y);

	return;
}

void TMessageBox::SetText(const char* _s)
{
	height_string = app.Font()->normal.GetHeight();

	/* On parse le message pour le découper en différentes lignes */
	char s[MSGBOX_MAXWIDTH + 20 + 2];
	for(uint i=0;;)
	{
		if(*_s == '\n' || ((i > MSGBOX_MAXWIDTH) && *_s == ' ') || (i > (MSGBOX_MAXWIDTH+20)) || !(*_s))
		{ /* Retour à la ligne. Si ça dépasse le MSGBOX_MAXWIDTH lettres, on laisse une chance
		   * à un caractère ' ' ou '\n' de s'interposer pour couper proprement. A partir de
		   * MSGBOX_MAXWIDTH + 20 on coupe net.
		   */
		    s[i] = '\0';
			message.push_back(std::string(s));
			i=0;

			h += height_string;
			uint yw = app.Font()->normal.GetWidth(s) + 50;
			if(w < yw) w = yw;

			if(*_s == '\n')
				_s++; /* Seulement une fois, pour retourner à la ligne si il y en a un autre */
			while(*_s == ' ') _s++;
			if(!*_s) break;
		}
		else
			s[i++] = *_s++;
	}
}

void TMessageBox::SetEdit()
{
	if(Edit) delete Edit;
	h += 10;
	Edit = new TEdit(x+15,y+h,w-15-20);
	Edit->Init();
	Edit->SetFocus();
	h += EDIT_HEIGHT;
}

void TMessageBox::Init(const char* s)
{
	h = 0;
	w = 0;
	SetText(s);
	if(b & HAVE_EDIT)
		SetEdit();
	SetButtons();

        SDL_Rect r_back = {0,0,w,h};

        background = SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_OPENGL, w, h,
                                                32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
        SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));
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
			                                  ButtonList[i].label);
			bt->Tag = ButtonList[i].flag;
			boutons.push_back(bt);
			tmpw += ButtonList[i].w + 1;
			if(tmph < ButtonList[i].h) tmph = ButtonList[i].h;
		}

	if(w < (tmpw-x + 20)) w = tmpw - x + 20;
	h += tmph + 5;
}
