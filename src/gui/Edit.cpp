/* src/gui/TEdit.cpp - TEdit GUI
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
 *
 * $Id$
 */

#include "tools/Font.h"
#include "tools/Images.h"
#include "Edit.h"
#include <SDL.h>

TEdit::TEdit (Font* f, int _x, int _y, uint _width, uint _maxlen, char* av, bool _show_bg)
  : TComponent(_x, _y, _width, f->GetHeight()), show_background(_show_bg), caret(0), have_redraw(true), font(f)
{
  assert(font);

  EDIT_HEIGHT = f->GetHeight();
  first_char = 0;
  maxlen = _maxlen;
  focus = false;
  chaine = "";

  background = NULL;
  edit = 0;
  avail_chars = av;
}

TEdit::~TEdit()
{
	if ( background)
		SDL_FreeSurface( background);
	if(edit)
		SDL_FreeSurface(edit);
}

void TEdit::Init()
{
	if ( background)
		SDL_FreeSurface( background);
	if(edit)
		SDL_FreeSurface(edit);

	visible_len = ((w) / font->GetWidth("A"));
	
	if(!show_background) return;
	
	SDL_Rect r_back = {0,0,w,h};
	
	background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
						32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
	SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));
}

void TEdit::SetFocus()
{
	TComponent::SetFocus();
	have_redraw = true;
}

void TEdit::DelFocus()
{
	TComponent::DelFocus();
	have_redraw = true;
}

void TEdit::Draw (int m_x, int m_y)
{
	if(have_redraw)
		Redraw();

	if(background)
	{
		SDL_Rect r_back = {x,y,background->w,background->h};
		SDL_BlitSurface( background, NULL, Window(), &r_back);
	}

	if(!edit) return;

	SDL_Rect dst_rect;
	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.w = edit->w;
	dst_rect.h = edit->h;

	SDL_BlitSurface(edit,NULL,Window(), &dst_rect);
}

void TEdit::SetString(std::string s)
{
	chaine = s;
	caret = s.size();
	first_char = caret > visible_len ? caret - visible_len : 0;
}

void TEdit::Redraw()
{
	if(!focus && chaine.empty())
	{
		edit = 0;
		return;
	}

	std::string substring;

	if(chaine.size() > visible_len)
		substring = chaine.substr(first_char, first_char+visible_len);
	else
		substring = chaine;

	if(edit)
		SDL_FreeSurface(edit);

	/* Le " " est nécessaire sinon il se peut que la surface soit trop petite et que caret_x en sorte */
	edit = TTF_RenderText_Blended(&(font->GetTTF()), std::string(substring + " ").c_str(), black_color);

	if(Focused())
	{
		uint caret_x = font->GetWidth(substring.substr(0, caret-first_char));
		SLOCK(edit);
		DrawLine(edit, caret_x, 1, caret_x, h-2,
		        SDL_MapRGB(edit->format, black_color.r, black_color.g, black_color.b));
		SUNLOCK(edit);
	}
	have_redraw = false;
}

void TEdit::PressKey(SDL_keysym key)
{
	if(!focus) return;

	switch(key.sym)
	{
		case SDLK_BACKSPACE:
			if(chaine.size() > 0 && caret > 0)
			{
				if(caret == first_char)
					first_char--;
				caret--;
				chaine.erase(chaine.begin() + caret);
				have_redraw = true;
			}
			break;
		case SDLK_DELETE:
			if(chaine.size() > 0 && caret <= chaine.size())
			{
				chaine.erase(chaine.begin() + caret);
				have_redraw = true;
			}
			break;
		case SDLK_LEFT:
			if(caret > 0)
			{
				if(caret == first_char)
					first_char--;
				caret--;
				have_redraw = true;
			}
			break;
		case SDLK_RIGHT:
			if(caret < chaine.size())
			{
				if(caret == visible_len + first_char)
					first_char++;
				caret++;
				have_redraw = true;
			}
			break;
		case SDLK_HOME:
			caret = 0;
			first_char = 0;
			have_redraw = true;
			break;
		case SDLK_END:
			caret = chaine.size();
			first_char = caret > visible_len ? caret - visible_len : 0;
			have_redraw = true;
			break;
		default:
		{
			if(maxlen && chaine.size() >= maxlen)
				return;
			char c = 0;
			if(key.unicode)
				c = key.unicode;
			else if(key.sym >= SDLK_a && key.sym <= SDLK_z)
			{
				if(key.mod & KMOD_SHIFT)
					c = 'A' + key.sym - SDLK_a;
				else
					c = 'a' + key.sym - SDLK_a;
			}
			else if(key.sym >= SDLK_0 && key.sym <= SDLK_9)
				c = '0' + key.sym - SDLK_0;

			if(c && (!avail_chars || strchr(avail_chars, c)))
			{
				chaine.insert(chaine.begin() + caret, c);
				if(caret == visible_len + first_char)
					first_char++;
				caret++;
				have_redraw = true;
			}
			break;
		}
	}
}
