/* src/tools/Font.h - Header of Font.cpp
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
#include <map>
#include "Images.h"

extern SDL_Color white_color;
extern SDL_Color black_color;
extern SDL_Color red_color;
extern SDL_Color gray_color;
extern SDL_Color green_color;
extern SDL_Color brown_color;
extern SDL_Color blue_color;
extern SDL_Color orange_color;

class Font
{
  typedef std::map<std::string, ECImage>::value_type
    txt_sample;
  typedef std::map<std::string, ECImage>::iterator
    txt_iterator;

  std::map<std::string, ECImage> surface_text_table;
  TTF_Font *m_font;
  void Write(int x, int y, ECImage &surface);

public:
  Font(int size);
  ~Font();
  
  bool Load (const std::string& filename, int size);
  TTF_Font& GetTTF() { return *m_font; }

  void WriteLeft (int x, int y, const std::string &txt, const SDL_Color &color);
  void WriteLeftBottom (int x, int y, const std::string &txt, const SDL_Color &color);
  void WriteRight (int x, int y, const std::string &txt, const SDL_Color &color);
  void WriteCenterTop (int x, int y, const std::string &txt, const SDL_Color &color);
  void WriteCenter (int x, int y, const std::string &txt, const SDL_Color &color);
  
  int GetWidth (const std::string &txt);
  int GetHeight ();
  int GetHeight (const std::string &txt);

  ECImage Render(const std::string &txt, const SDL_Color &color, bool cache=false);
  ECImage CreateSurface(const std::string &txt, const SDL_Color &color);
};

class Fonts
{
public:
  Fonts();
  Font huge;
  Font large;
  Font big;
  Font normal;
  Font small;
  Font tiny;
};

//-----------------------------------------------------------------------------
#endif
