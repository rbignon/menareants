/* src/gui/ColorEdit.cpp - To select a color in a list.
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

#include "ColorEdit.h"
#include "tools/Maths.h"
#include "Resources.h"
#include "tools/Color.h"

Color color_eq[] = {
	/* {COLOR_NONE  */          white_color,
	/* COLOR_GRAY   */          gray_color,
	/* COLOR_BLUE   */          fblue_color,
	/* COLOR_RED    */          fred_color,
	/* COLOR_GREEN  */          green_color,
	/* COLOR_VIOLET */          violet_color,
	/* COLOR_BROWN  */          fbrown_color,
	/* COLOR_ORANGE */          orange_color,
	/* COLOR_CYAN   */          cyan_color,
	/* COLOR_BLACK  */          black_color
};

TColorEdit::TColorEdit(Font* f, std::string _label, int _x, int _y, uint _width, int _defvalue)
	: TSpinEdit(f, _label, _x, _y, _width, COLOR_NONE, COLOR_MAX-1, 1, _defvalue)
{
	imgx = 0;
}

void TColorEdit::Init()
{
  uint max_value_w = 20;
  uint margin = 10;

  m_plus.SetXY(x+w-10,y);
  m_minus.SetXY(x+w-max_value_w-10-2*margin,y);
  MyComponent(&m_plus);
  MyComponent(&m_minus);
  m_plus.SetImage (new ECSprite(Resources::UpButton(), Window()));
  m_minus.SetImage (new ECSprite(Resources::DownButton(), Window()));

  MyComponent(&txt_label);

  txt_label.SetFontColor(font, color);
  txt_label.SetXY(X(), Y());

  imgx = m_minus.X() + m_minus.Width() + 5;

  img.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE, 20, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));

  SetValue(value, true);
}

bool TColorEdit::SetValue(int _value, bool first)
{
  _value = BorneLong(_value, min, max);

  if(_value == value && !first) return false;
  value = _value;

  Color color = color_eq[value];

  SDL_Rect r_back = {0,0,img.GetWidth(),img.GetHeight()};

  if(value)
    img.Fill(img.MapColor(color));
  else
    img.FillRect(r_back, img.MapRGBA(255,255,255, 0));

  return true;
}

void TColorEdit::SetXY (int px, int py) { x = px; y = py; Init(); }

void TColorEdit::Draw (int mouse_x, int mouse_y)
{
	txt_label.Draw(mouse_x, mouse_y);

	if(!img.IsNull())
	{
		SDL_Rect r_back = {imgx,y,img.GetWidth(),img.GetHeight()};
		Window()->Blit(img, &r_back);
	}

	if(enabled)
	{
		m_minus.Draw (mouse_x, mouse_y);
		m_plus.Draw (mouse_x, mouse_y);
	}
}
