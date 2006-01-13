/* src/gui/SpinEdit.cpp - To increment or decrement a comptor
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

#include "SpinEdit.h"
#include "tools/Font.h"
#include "tools/Maths.h"
#include "Resources.h"
#include "Main.h"
#include <iostream>
#include <sstream>

TSpinEdit::TSpinEdit(std::string _label, uint _x, uint _y, uint _width, int _min, int _max, uint _step,
                     int _defvalue)
	: TComponent(_x, _y, _width, SPINEDIT_HEIGHT), min(_min), max(_max), step(_step),
	  label(_label)
{
	/* Sécurités */
	if(max < min) max = min;
	if(value < _min) value = _min;
	else if(value > _max) value = _max;

	value = _defvalue;

	background = NULL;
	txt_label = NULL;
	txt_value = NULL;
	m_plus = NULL;
	m_minus = NULL;
	font = &small_font;
	color = white_color;
}

TSpinEdit::~TSpinEdit()
{
  if ( background)
     SDL_FreeSurface( background);
  if(txt_label) delete txt_label;
  if(txt_value) delete txt_value;
  if(m_plus) delete m_plus;
  if(m_minus) delete m_minus;
}

void TSpinEdit::Init()
{
  if ( background)
    SDL_FreeSurface( background);

  // Load images
  std::ostringstream max_value_s;
  max_value_s << max;
  uint max_value_w = font->GetWidth(max_value_s.str());

  uint margin = 5;

  if(m_plus) delete m_plus;
  if(m_minus) delete m_minus;

  m_plus = new TButton (x+w-5,y,5,10);
  m_minus = new TButton (x+w-max_value_w-5-2*margin,y,5,10);

  m_plus->SetImage (new ECSprite(Resources::UpButton(), app.sdlwindow));
  m_minus->SetImage (new ECSprite(Resources::DownButton(), app.sdlwindow));

  if(txt_label) delete txt_label;
  if(txt_value) delete txt_value;

  uint center = (m_plus->GetX() +5 + m_minus->GetX() )/2;
  txt_label = new TLabel(x, y, label, color, font);
  txt_value = new TLabel(center, y, "", color, font);
  SetValue(value, true);

  SDL_Rect r_back = {0,0,w,h};

  background = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
  SDL_FillRect( background, &r_back, SDL_MapRGBA( background->format,255, 255, 255, 255*3/10));

}

bool TSpinEdit::SetValue(int _value, bool first)
{
  _value = BorneLong(_value, min, max);

  if(_value == value && !first) return false;
  value = _value;

  std::ostringstream value_s;
  value_s << value ;

  std::string s(value_s.str());
  txt_value->SetCaption(s);

  return true;
}

void TSpinEdit::Draw (uint mouse_x, uint mouse_y)
{
  txt_label->Draw(mouse_x, mouse_y);

  m_minus->Draw (mouse_x, mouse_y);
  m_plus->Draw (mouse_x, mouse_y);

  txt_value->Draw(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------

bool TSpinEdit::Clic (uint mouse_x, uint mouse_y)
{
  if (m_minus->Test(mouse_x, mouse_y))
    return SetValue(value - step);
  else if (m_plus->Test(mouse_x, mouse_y))
    return SetValue(value + step);

  return false;
}

void TSpinEdit::SetColorFont(SDL_Color new_color, Font* new_font)
{
	color = new_color;
	font = new_font;
}
