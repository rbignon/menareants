/* src/gui/SpinEdit.cpp - To increment or decrement a comptor
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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
	  min(_min), max(_max), step(_step), color(white_color), font(f), on_change(0)
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

  m_plus.SetXY(X()+Width()-10,Y());
  m_minus.SetXY(X()+Width()-max_value_w-10-2*margin,Y());
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

  SetWantRedraw();

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
		else if(SetValue(new_value))
		{
			if(on_change)
				(*on_change) (this);
			return true;
		}
		else
			return false;
	}
	return false;
}

void TSpinEdit::Draw (const Point2i& mouse)
{
	txt_label.Draw(mouse);

	if(Enabled())
	{
		m_minus.Draw (mouse);
		m_plus.Draw (mouse);
	}

	txt_value.Draw(mouse);
}

//-----------------------------------------------------------------------------

bool TSpinEdit::Clic (const Point2i& mouse, int button)
{
  if(!Enabled() || !Mouse(mouse)) return false;

  if(button == SDL_BUTTON_WHEELUP || button == SDL_BUTTON_RIGHT || m_plus.Test(mouse, button))
    return ChangeValueByClick(true);
  else if(button == SDL_BUTTON_WHEELDOWN || button == SDL_BUTTON_LEFT
          /* inutile du coup, vu que Test() nécessite button == SDL_BUTTON_LEFT|| m_minus.Test(mouse, button) */
         )
    return ChangeValueByClick(false);

  return false;
}

void TSpinEdit::SetColorFont(Color new_color, Font* new_font)
{
	color = new_color;
	font = new_font;
	Init();
}

void TSpinEdit::SetXY (int px, int py) { TComponent::SetXY(px, py); Init(); }

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
