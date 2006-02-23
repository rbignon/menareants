/* src/tools/Font.cpp - Fonts from Wormux
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
#include <SDL/SDL_image.h>
#include <SDL/SDL_video.h>
#include <iostream>
#include <string>
#include "Main.h"
#include "Font.h"
#include "Outils.h"
#include "Defines.h"
#include "Debug.h"
//-----------------------------------------------------------------------------
SDL_Color white_color = { 0xFF, 0xFF, 0xFF, 0 };
SDL_Color black_color = { 0x00, 0x00, 0x00, 0 };
SDL_Color red_color =   { 0xFF, 0x00, 0x04, 0 };
SDL_Color gray_color =  { 0x56, 0x56, 0x56, 0 };
SDL_Color green_color = { 0x00, 0x5A, 0x00, 0 };
SDL_Color brown_color = { 0x6A, 0x4C, 0x3C, 0 };
SDL_Color blue_color =  { 0x00, 0x00, 0xFF, 0 };
SDL_Color orange_color= { 0xff, 0x8c, 0x45, 0 };

//-----------------------------------------------------------------------------

Font::Font(int size){ 
  m_font = NULL;
  std::string vera_ttf = "Vera.ttf";
  std::string filename  = PKGDATADIR_FONTS + vera_ttf;
  bool ok = Load(filename, size);
  
  if( !ok )
    Debug(W_ERR, "Error during initialisation of a font!");
}

Font::~Font(){
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

bool Font::Load (const std::string& filename, int size) {
  bool ok = false;

  if( FichierExiste(filename) ){
      m_font = TTF_OpenFont(filename.c_str(), size);
      ok = (m_font != NULL);
  }
  
  if( !ok ){
      std::cout << "Error: Font " << filename << " can't be found!" << std::endl;
      return false;
  }
  
  TTF_SetFontStyle(m_font, TTF_STYLE_NORMAL);

  return true;
}

void Font::Write(int x, int y, ECImage &surface)
{
  SDL_Rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.h = surface.GetHeight();
  dst_rect.w = surface.GetWidth();

  SDL_BlitSurface(surface.Img,NULL,app.sdlwindow, &dst_rect);
}

void Font::WriteLeft (int x, int y, const std::string &txt,  const SDL_Color &color){
  ECImage surface( Render(txt, color, true) );
  Write(x, y, surface);
}

void Font::WriteLeftBottom (int x, int y, const std::string &txt,
     const SDL_Color &color){
  ECImage surface( Render(txt, color, true) );
  Write(x, y - surface.GetHeight(), surface);
}

void Font::WriteRight (int x, int y, const std::string &txt,
        const SDL_Color &color){
  ECImage surface( Render(txt, color, true) );
  Write(x - surface.GetWidth(), y, surface);
}

void Font::WriteCenter (int x, int y, const std::string &txt,
 const SDL_Color &color){
  ECImage surface( Render(txt, color, true) );
  Write( x - surface.GetWidth()/2, y - surface.GetHeight(), surface);
}

void Font::WriteCenterTop (int x, int y, const std::string &txt,
    const SDL_Color &color){
  ECImage surface( Render(txt, color, true) );
  Write( x - surface.GetWidth() / 2, y, surface);
}

ECImage Font::CreateSurface(const std::string &txt, const SDL_Color &color){
  return ECImage( TTF_RenderText_Blended(m_font, txt.c_str(), color) );
}

ECImage Font::Render(const std::string &txt, const SDL_Color &color, bool cache){
  ECImage surface;
  
/*  if( cache ){
    txt_iterator p = surface_text_table.find(txt);
    if( p == surface_text_table.end() ){ 
      if( surface_text_table.size() > 5 ){
        //SDL_FreeSurface( surface_text_table.begin()->second );
        surface_text_table.erase( surface_text_table.begin() );
      }
      surface = CreateSurface(txt, color);
      surface_text_table.insert( txt_sample(txt, surface) );
    } else {
      txt_iterator p = surface_text_table.find( txt );
      surface = p->second;
    }
  } else*/
    surface = CreateSurface(txt, color);
  
  assert( surface.Img );
  return surface;
}

int Font::GetWidth (const std::string &txt){ 
  int width=-1;
  
  TTF_SizeText(m_font, txt.c_str(), &width, NULL);

  return width;
}

int Font::GetHeight (){ 
  return TTF_FontHeight(m_font);
}

int Font::GetHeight (const std::string &str){ 
  int height=-1;
  
  TTF_SizeText(m_font, str.c_str(), NULL, &height);

  return height;
}

Fonts::Fonts() :
  huge(40),
  large(32),
  big(24),
  normal(16),
  small(12),
  tiny(8)
{}
