/* src/tools/Font.cpp - Fonts from Wormux
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
/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************/
#include <SDL/SDL_image.h>
#include <SDL/SDL_video.h>
#include <iostream>
#include <string>
#include "../Main.h"
#include "Font.h"
#include "../Defines.h"
//-----------------------------------------------------------------------------
SDL_Color white_color = { 0xFF, 0xFF, 0xFF, 0 };
SDL_Color black_color = { 0x00, 0x00, 0x00, 0 };
//-----------------------------------------------------------------------------

Font huge_font;
Font large_font;
Font big_font;
Font normal_font;
Font small_font;
Font tiny_font;

void Font::InitAllFonts()
{
  huge_font.Load(PKGDATADIR_FONTS"Vera.ttf", 40);
  large_font.Load(PKGDATADIR_FONTS"Vera.ttf", 32);
  big_font.Load(PKGDATADIR_FONTS"Vera.ttf", 24);
  normal_font.Load(PKGDATADIR_FONTS"Vera.ttf", 16);
  small_font.Load(PKGDATADIR_FONTS"Vera.ttf", 12);
  tiny_font.Load(PKGDATADIR_FONTS"Vera.ttf", 8);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Font::Font()
{
  m_font = NULL;
}

//-----------------------------------------------------------------------------

Font::~Font()
{
  if (m_font != NULL) {
    TTF_CloseFont(m_font);
    m_font = NULL;
  }
}

//-----------------------------------------------------------------------------

void Font::Load (const std::string& font_name, int size)
{
  //assert (m_font == NULL);
  m_font = TTF_OpenFont(font_name.c_str(), size);
  //assert (m_font != NULL);
  TTF_SetFontStyle(m_font,TTF_STYLE_NORMAL);
}
//-----------------------------------------------------------------------------

void Font::WriteLeft (int x, int y, const std::string &txt,
		      SDL_Color color)
{
  //assert (m_font != NULL);
  SDL_Surface * text_surface = Render(txt.c_str(), color);
  SDL_Rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.h = text_surface->h;
  dst_rect.w = text_surface->w;

  SDL_BlitSurface(text_surface,NULL,app.sdlwindow, &dst_rect);
  SDL_FreeSurface(text_surface);
}

// //-----------------------------------------------------------------------------

void Font::WriteLeftBottom (int x, int y, const std::string &txt,
			    SDL_Color color)
{
  //assert (m_font != NULL);
  SDL_Surface * text_surface = Render(txt.c_str(), color);
  SDL_Rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y - GetHeight();
  dst_rect.h = text_surface->h;
  dst_rect.w = text_surface->w;

  SDL_BlitSurface(text_surface,NULL,app.sdlwindow, &dst_rect);
  SDL_FreeSurface(text_surface);
}

// //-----------------------------------------------------------------------------

void Font::WriteRight (int x, int y, const std::string &txt,
		       SDL_Color color)
{
  //assert (m_font != NULL);
  SDL_Surface * text_surface = Render(txt.c_str(), color);
  SDL_Rect dst_rect;
  dst_rect.x = x - GetWidth(txt);
  dst_rect.y = y;
  dst_rect.h = text_surface->h;
  dst_rect.w = text_surface->w;

  SDL_BlitSurface(text_surface, NULL, app.sdlwindow, &dst_rect);
  SDL_FreeSurface(text_surface);
}

// //-----------------------------------------------------------------------------

void Font::WriteCenter (int x, int y, const std::string &txt,
			SDL_Color color)
{
  //assert (m_font != NULL);
  SDL_Surface * text_surface = Render(txt.c_str(), color);
  SDL_Rect dst_rect;
  dst_rect.x = x - GetWidth(txt)/2;
  dst_rect.y = y - GetHeight()/2;
  dst_rect.h = text_surface->h;
  dst_rect.w = text_surface->w;

  SDL_BlitSurface(text_surface, NULL, app.sdlwindow, &dst_rect);
  SDL_FreeSurface(text_surface);
}

// //-----------------------------------------------------------------------------

void Font::WriteCenterTop (int x, int y, const std::string &txt,
			   SDL_Color color)
{
  //assert (m_font != NULL);
  SDL_Surface * text_surface = Render(txt.c_str(), color);
  SDL_Rect dst_rect;
  dst_rect.x = x - GetWidth(txt)/2;
  dst_rect.y = y;
  dst_rect.h = text_surface->h;
  dst_rect.w = text_surface->w;

  SDL_BlitSurface(text_surface, NULL, app.sdlwindow, &dst_rect);
  SDL_FreeSurface(text_surface);
}

//-----------------------------------------------------------------------------

SDL_Surface * Font::Render(const std::string &txt, SDL_Color color)
{
  //assert (m_font != NULL);
  SDL_Surface * surface = TTF_RenderUTF8_Blended(m_font, txt.c_str(),
						 color); //, black_color);
  //assert (surface != NULL);
  return surface;
}

//-----------------------------------------------------------------------------

int Font::GetWidth (const std::string &txt)
{
  //assert (m_font != NULL);
  int width=-1;
  TTF_SizeUTF8(m_font, txt.c_str(), &width, NULL);
  return width;
}

//-----------------------------------------------------------------------------

int Font::GetHeight ()
{
  //assert (m_font != NULL);
  return TTF_FontHeight(m_font);
}

//-----------------------------------------------------------------------------

int Font::GetHeight (const std::string &str)
{
  int height=-1;
  TTF_SizeUTF8(m_font, str.c_str(), NULL, &height);
  return height;
}

//-----------------------------------------------------------------------------

