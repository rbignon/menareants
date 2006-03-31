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

TSpinEdit::TSpinEdit(std::string _label, int _x, int _y, uint _width, int _min, int _max, uint _step,
                     int _defvalue)
	: TComponent(_x, _y, _width, SPINEDIT_HEIGHT), min(_min), max(_max), step(_step),
	  label(_label)
{
	/* Sécurités */
	if(max < min) max = min;
	if(value < min) value = min;
	else if(value > max) value = max;

	value = _defvalue;

	txt_label = NULL;
	txt_value = NULL;
	m_plus = NULL;
	m_minus = NULL;
	font = &app.Font()->small;
	color = white_color;
}

TSpinEdit::~TSpinEdit()
{
  if(txt_label) delete txt_label;
  if(txt_value) delete txt_value;
  if(m_plus) delete m_plus;
  if(m_minus) delete m_minus;
}

void TSpinEdit::Init()
{
  // Load images
  std::ostringstream max_value_s;
  max_value_s << max;
  uint max_value_w = font->GetWidth(max_value_s.str());
  uint margin = 5;

  /* Boutons */
  if(m_plus) delete m_plus;
  if(m_minus) delete m_minus;

  m_plus = new TButton (x+w-5,y,5,10);
  m_minus = new TButton (x+w-max_value_w-5-2*margin,y,5,10);

  /* Images */
  /* Pas besoin de delete, ECImage le fait */
  m_plus->SetImage (new ECSprite(Resources::UpButton(), app.sdlwindow));
  m_minus->SetImage (new ECSprite(Resources::DownButton(), app.sdlwindow));

  /* Label */
  if(txt_label) delete txt_label;
  if(txt_value) delete txt_value;

  uint center = (m_plus->X() +5 + m_minus->X() )/2;
  txt_label = new TLabel(x, y, label, color, font);
  txt_value = new TLabel(center, y, "", color, font);
  SetValue(value, true);
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

bool TSpinEdit::ChangeValueByClick(bool up)
{
	int new_value = up ? value + step : value - step;
	while(1)
	{
		if(new_value > max) new_value = max;
		else if(new_value < min) new_value = min;
		
		std::vector<int>::iterator it;
		for(it = bad_values.begin(); it != bad_values.end() && *it != new_value; ++it);
		if(it != bad_values.end())
		{
			if(new_value == max || new_value == min) return false;
			new_value = up ? new_value + step : new_value - step;
			continue;
		}
		else
			return SetValue(new_value);
	}
	return false;
}

void TSpinEdit::Draw (int mouse_x, int mouse_y)
{
	if(txt_label)
		txt_label->Draw(mouse_x, mouse_y);

	if(enabled)
	{
		if(m_minus)
			m_minus->Draw (mouse_x, mouse_y);
		if(m_plus)
			m_plus->Draw (mouse_x, mouse_y);
	}

	if(txt_value)
		txt_value->Draw(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------

bool TSpinEdit::Clic (int mouse_x, int mouse_y)
{
  if(!m_minus || !m_plus || !enabled) return false;

  if (m_minus->Test(mouse_x, mouse_y))
    return ChangeValueByClick(false);
  else if (m_plus->Test(mouse_x, mouse_y))
    return ChangeValueByClick(true);

  return false;
}

void TSpinEdit::SetColorFont(SDL_Color new_color, Font* new_font)
{
	color = new_color;
	font = new_font;
	Init();
}

void TSpinEdit::SetXY (int px, int py) { x = px; y = py; Init(); }

void TSpinEdit::SetMax(int _max)
{
	max = _max;
	if(max < min) max = min;
	if(value > max) value = max;
}

void TSpinEdit::SetMin(int _min)
{
	min = _min;
	if(max < min) max = min;
	if(value < min) value = min;
}

void TSpinEdit::AddBadValue(int i)
{
	for(std::vector<int>::iterator it = bad_values.begin(); it != bad_values.end(); ++it)
		if(*it == i)
			return;
	bad_values.push_back(i);
	return;
}

void TSpinEdit::DelBadValue(int i)
{
	for (std::vector<int>::iterator it = bad_values.begin(); it != bad_values.end(); )
	{
		if (*it == i)
		{
			it = bad_values.erase(it);
			return;
		}
		else
			++it;
	}
}
