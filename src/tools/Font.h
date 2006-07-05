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
 *
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
 
#ifndef EC_FONT_H
#define EC_FONT_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <map>
#include "Color.h"
#include "Images.h"

class Font
{
private:
  typedef std::map<std::string, ECImage>::value_type
    txt_sample;
  typedef std::map<std::string, ECImage>::iterator
    txt_iterator;

  static const int FONT_SIZE[];
  static Font* FONT_ARRAY[6];

  std::map<std::string, ECImage> surface_text_table;
  TTF_Font *m_font;
  void Write(int x, int y, ECImage &surface);

  Font(int size);

public:

  enum
  {
    Huge, Large, Big, Normal, Small, Tiny
  };

  // type: defined as static consts above
  static Font* GetInstance(int type);

  ~Font();

  bool Load (const std::string& filename, int size);
  TTF_Font& GetTTF() { return *m_font; }

  void WriteLeft(int x, int y, const std::string &txt, const Color &color);
  void WriteLeftBottom(int x, int y, const std::string &txt, const Color &color);
  void WriteRight(int x, int y, const std::string &txt, const Color &color);
  void WriteCenterTop(int x, int y, const std::string &txt, const Color &color);
  void WriteCenter(int x, int y, const std::string &txt, const Color &color);
  
  int GetWidth(const std::string &txt);
  int GetHeight();
  int GetHeight(const std::string &txt);

  ECImage Render(const std::string &txt, const Color &color, bool cache=false);
  ECImage CreateSurface(const std::string &txt, const Color &color);
};

#endif /* EC_FONT_H */
