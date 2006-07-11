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
#include <iostream>
#include <sstream>

TSpinEdit::TSpinEdit(Font* f, std::string _label, int _x, int _y, uint _width, int _min, int _max, uint _step,
                     int _defvalue)
	: TComponent(_x, _y, _width, SPINEDIT_HEIGHT), m_plus(_x,_y,10,10), m_minus(_x,_y,10,10),
	  txt_label(_x, _y, _label, white_color, f), txt_value(_x,_y,"", white_color, f),
	  min(_min), max(_max), step(_step), color(white_color), font(f)
{
	/* Sécurités */
	if(max < min) max = min;
	if(_defvalue < min) _defvalue = min;
	else if(_defvalue > max) _defvalue = max;

	value = _defvalue;
}

void TSpinEdit::Init()
{
  // Load images
  std::ostringstream max_value_s;
  max_value_s << max;
  uint max_value_w = font->GetWidth(max_value_s.str());
  uint margin = 5;

  m_plus.SetXY(x+w-10,y);
  m_minus.SetXY(x+w-max_value_w-10-2*margin,y);
  MyComponent(&m_plus);
  MyComponent(&m_minus);
  m_plus.SetImage (new ECSprite(Resources::UpButton(), Window()));
  m_minus.SetImage (new ECSprite(Resources::DownButton(), Window()));

  MyComponent(&txt_label);
  MyComponent(&txt_value);

  txt_label.SetFontColor(font, color);
  txt_label.SetXY(X(), Y());
  txt_value.SetFontColor(font, color);
  txt_value.SetY(Y());

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
  txt_value.SetCaption(s);

  uint center = (m_plus.X() +10 + m_minus.X() )/2 - font->GetWidth(s)/2;
  txt_value.SetX(center);

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
	txt_label.Draw(mouse_x, mouse_y);

	if(enabled)
	{
		m_minus.Draw (mouse_x, mouse_y);
		m_plus.Draw (mouse_x, mouse_y);
	}

	txt_value.Draw(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------

bool TSpinEdit::Clic (int mouse_x, int mouse_y)
{
  if(!enabled) return false;

  if (m_minus.Test(mouse_x, mouse_y))
    return ChangeValueByClick(false);
  else if (m_plus.Test(mouse_x, mouse_y))
    return ChangeValueByClick(true);

  return false;
}

void TSpinEdit::SetColorFont(Color new_color, Font* new_font)
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
