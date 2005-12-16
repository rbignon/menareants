/* src/tools/Font.h - Header of Font.cpp
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

#ifndef FONT_H
#define FONT_H
//-----------------------------------------------------------------------------
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

//-----------------------------------------------------------------------------

class Font
{
public:
  TTF_Font *m_font;

public:
  Font();
  ~Font();
  void Load (const std::string& resource_id, int size);
  void WriteLeft (int x, int y, const std::string &txt, SDL_Color color);
  void WriteLeftBottom (int x, int y, const std::string &txt, SDL_Color color);
  void WriteRight (int x, int y, const std::string &txt, SDL_Color color);
  void WriteCenterTop (int x, int y, const std::string &txt, SDL_Color color);
  void WriteCenter (int x, int y, const std::string &txt, SDL_Color color);
  int GetWidth (const std::string &txt);
  int GetHeight ();
  int GetHeight (const std::string &txt);
  SDL_Surface * Render(const std::string &txt, SDL_Color color);

  static void InitAllFonts();
 };

extern SDL_Color white_color;
extern SDL_Color black_color;
extern SDL_Color red_color;
extern SDL_Color gray_color;
extern SDL_Color green_color;

extern Font huge_font;
extern Font large_font;
extern Font big_font;
extern Font normal_font;
extern Font small_font;
extern Font tiny_font;

//-----------------------------------------------------------------------------
#endif
