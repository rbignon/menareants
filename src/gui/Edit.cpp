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
#include <assert.h>
#include <SDL.h>

TEdit::TEdit (Font* f, int _x, int _y, uint _width, uint _maxlen, char* av, bool _show_bg)
  : TComponent(_x, _y, _width, f->GetHeight()), show_background(_show_bg), caret(0), have_redraw(true), font(f),
    color(black_color)
{
	assert(font);

	EDIT_HEIGHT = f->GetHeight();
	first_char = 0;
	maxlen = _maxlen;
	DelFocus();
	chaine = "";

	avail_chars = av;
}

void TEdit::Init()
{
	visible_len = (Width() / font->GetWidth("A"));

	if(!show_background) return;

	SDL_Rect r_back = {0,0,Width(), Height()};

	background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, Width(), Height(),
						32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
	background.FillRect(r_back, background.MapColor(BoxColor));
}

void TEdit::SetFocus()
{
	TComponent::SetFocus();
	have_redraw = true;
	SetWantRedraw();
}

void TEdit::DelFocus()
{
	TComponent::DelFocus();
	have_redraw = true;
	SetWantRedraw();
}

void TEdit::Draw (const Point2i& mouse)
{
	if(have_redraw)
		Redraw();

	if(!background.IsNull())
		Window()->Blit(background, position);

	if(edit.IsNull()) return;

	Window()->Blit(edit, position);
}

void TEdit::SetString(std::string s)
{
	chaine = s;
	caret = s.size();
	first_char = caret > visible_len ? caret - visible_len : 0;
	have_redraw = true;
	SetWantRedraw();
}

void TEdit::Redraw()
{
	if(!Focused() && chaine.empty())
	{
		edit.SetImage(0);
		return;
	}

	std::string substring;

	if(chaine.size() > visible_len)
		substring = chaine.substr(first_char, first_char+visible_len);
	else
	{
		substring = chaine;
		first_char = 0;
		caret = chaine.size();
	}

	/* Le " " est nécessaire sinon il se peut que la surface soit trop petite et que caret_x en sorte */
	edit = font->CreateSurface(substring + " ", color);

	if(Focused())
	{
		uint caret_x = substring.empty() ? 1 : font->GetWidth(substring.substr(0, caret-first_char));
		SLOCK(edit.Img);
		DrawLine(edit.Img, caret_x, 1, caret_x, Height()-2, edit.MapColor(color));
		SUNLOCK(edit.Img);
	}
	have_redraw = false;
}

void TEdit::PressKey(SDL_keysym key)
{
	if(!Focused()) return;

	switch(key.sym)
	{
		case SDLK_BACKSPACE:
			if(chaine.size() > 0 && caret > 0)
			{
				while(caret && (chaine[--caret] & 0xc0) == 0x80)
					chaine.erase(caret, 1);
				chaine.erase(caret, 1);
				if(first_char > 0)
					while(first_char && (chaine[--first_char] & 0xc0) == 0x80);
				have_redraw = true;
			}
			break;
		case SDLK_LEFT:
			if(caret > 0)
			{
				while(caret && (chaine[--caret] & 0xc0) == 0x80);
				if(caret < first_char)
					first_char = caret;
				have_redraw = true;
			}
			break;
		case SDLK_RIGHT:
			if(caret < chaine.size())
			{
				while((chaine[++caret] & 0xc0) == 0x80);
				if(caret > visible_len + first_char)
					first_char = caret - visible_len;
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
		case SDLK_DELETE:
		case SDLK_RETURN: break; /* On ne prend pas "entrée" comme un caractère */
		default:
		{
			if(maxlen && chaine.size() >= maxlen)
				return;
			if(key.unicode < ' ') return;

			std::string new_txt = chaine;
			if(key.unicode > 0)
			{
				if(key.unicode < 0x80 && (!avail_chars || strchr(avail_chars, (char)key.unicode))) // 1 byte char
					new_txt.insert(caret++, 1, (char)key.unicode);
				else if(avail_chars)
					break;
				else if (key.unicode < 0x800) // 2 byte char
				{
					new_txt.insert(caret++, 1, (char)(((key.unicode & 0x7c0) >> 6) | 0xc0));
					new_txt.insert(caret++, 1, (char)((key.unicode & 0x3f) | 0x80));
				}
				else // if (key.unicode < 0x10000) // 3 byte char
				{
					new_txt.insert(caret++, 1, (char)(((key.unicode & 0xf000) >> 12) | 0xe0));
					new_txt.insert(caret++, 1, (char)(((key.unicode & 0xfc0) >> 6) | 0x80));
					new_txt.insert(caret++, 1, (char)((key.unicode & 0x3f) | 0x80));
				}
			}
			else
			{
				char c;
				if(key.sym >= SDLK_a && key.sym <= SDLK_z)
				{
					if(key.mod & KMOD_SHIFT)
						c = 'A' + key.sym - SDLK_a;
					else
						c = 'a' + key.sym - SDLK_a;
				}
				else if(key.sym >= SDLK_0 && key.sym <= SDLK_9)
					c = '0' + key.sym - SDLK_0;
				else
					break;
				if(avail_chars && !strchr(avail_chars, c))
					break;

				new_txt.insert(caret++, 1, c);
			}

			chaine = new_txt;

			if(caret > visible_len + first_char)
			{
				first_char = caret - visible_len;
				while((chaine[first_char] & 0xc0) == 0x80) ++first_char;
			}
			have_redraw = true;
			break;
		}
	}
	SetWantRedraw();
}
