/* src/tools/Font.cpp - Fonts' tools
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

#include <SDL_image.h>
#include <SDL_video.h>
#include <iostream>
#include <string>
#include "Font.h"
#include "Color.h"
#include "Config.h"
#include "Debug.h"
#include "Video.h"
/*#include "../game/config.h"
#include "../include/app.h"
#include "../map/map.h"
#include "../tool/error.h"
#include "../tool/file_tools.h"*/

Font* Font::FONT_ARRAY[] = {NULL, NULL, NULL, NULL, NULL, NULL};

/*
 * Constants
 */
const int Font::FONT_SIZE[] = {40, 32, 24, 16, 14, 12};

Font* Font::GetInstance(int type)
{
  if (FONT_ARRAY[type] == NULL)
    FONT_ARRAY[type] = new Font(FONT_SIZE[type]);

  return FONT_ARRAY[type];
}

Font::Font(int size)
{
  m_font = NULL;
  bool ok = Load(Config::GetInstance()->ttf_file, size);

  if( !ok )
    throw ECExcept(VIName(size), "Error during initialisation of a font!");
}

Font::~Font()
{
  if( m_font != NULL ){
    TTF_CloseFont(m_font);
    m_font = NULL;
  }

  txt_iterator it;

  for( it = surface_text_table.begin();
       it != surface_text_table.end();
       ++it ){
    surface_text_table.erase(it->first);
  }
}

bool Font::Load (const std::string& filename, int size)
{
  bool ok = false;

  if( FichierExiste(filename) ){
      m_font = TTF_OpenFont(filename.c_str(), size);
      ok = (m_font != NULL);
  }

  if( !ok ){
      std::cerr << "Error: Font " << filename << " can't be found!" << std::endl;
      return false;
  }

  TTF_SetFontStyle(m_font, TTF_STYLE_NORMAL);

  return true;
}

void Font::Write(int x, int y, ECImage &surface)
{
  SDL_Rect pos;
  pos.x = x;
  pos.y = y;
  pos.h = surface.GetHeight();
  pos.w = surface.GetWidth();

  Video::GetInstance()->Window()->Blit(surface, &pos);
}

void Font::WriteLeft(int x, int y, const std::string &txt,  const Color &color)
{
  if(txt.empty()) return;
  ECImage surface(Render(txt, color, true));
  Write(x, y, surface);
}

void Font::WriteLeftBottom(int x, int y, const std::string &txt, const Color &color)
{
  if(txt.empty()) return;
  ECImage surface(Render(txt, color, true));
  Write(x, y - surface.GetHeight(), surface);
}

void Font::WriteRight(int x, int y, const std::string &txt, const Color &color)
{
  if(txt.empty()) return;
  ECImage surface(Render(txt, color, true));
  Write(x - surface.GetWidth(), y, surface);
}

void Font::WriteCenter (int x, int y, const std::string &txt, const Color &color)
{
  if(txt.empty()) return;
  ECImage surface(Render(txt, color, true));
  Write( x - surface.GetWidth()/2, y - surface.GetHeight()/2, surface);
}

void Font::WriteCenterTop(int x, int y, const std::string &txt, const Color &color)
{
	if(txt.empty()) return;
	ECImage surface(Render(txt, color, true));
	Write( x - surface.GetWidth() / 2, y, surface);
}

ECImage Font::CreateSurface(const std::string &txt, const Color &color)
{
  return ECImage( TTF_RenderText_Blended(m_font, txt.c_str(), color.GetSDLColor()) );
}

ECImage Font::Render(const std::string &txt, const Color &color, bool cache)
{
  ECImage surface;

  if( 0 )
  {
    txt_iterator p = surface_text_table.find(txt);
    if( p == surface_text_table.end() )
    {
      if( surface_text_table.size() > 5 )
        surface_text_table.erase( surface_text_table.begin() );

      surface = CreateSurface(txt, color);
      surface_text_table.insert( txt_sample(txt, surface) );
    }
    else
    {
      txt_iterator p = surface_text_table.find( txt );
      surface = p->second;
    }
  }
  else
    surface = CreateSurface(txt, color);

  assert( !surface.IsNull() );
  return surface;
}

int Font::GetWidth (const std::string &txt)
{
  int width=-1;

  TTF_SizeText(m_font, txt.c_str(), &width, NULL);

  return width;
}

int Font::GetHeight ()
{
  return TTF_FontHeight(m_font);
}

int Font::GetHeight (const std::string &str)
{
  int height=-1;

  TTF_SizeText(m_font, str.c_str(), NULL, &height);

  return height;
}
