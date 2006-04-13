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

SDL_Color *color_eq[] = {
	/* {COLOR_NONE  */          NULL,
	/* COLOR_GRAY   */          &gray_color,
	/* COLOR_BLUE   */          &fblue_color,
	/* COLOR_RED    */          &fred_color,
	/* COLOR_GREEN  */          &green_color,
	/* COLOR_WHITE  */          &fwhite_color,
	/* COLOR_BROWN  */          &fbrown_color,
	/* COLOR_ORANGE */          &orange_color,
	/* COLOR_MAX    */          NULL
};

TColorEdit::TColorEdit(Font* f, std::string _label, int _x, int _y, uint _width, int _defvalue)
	: TSpinEdit(f, _label, _x, _y, _width, COLOR_NONE, COLOR_MAX-1, 1, _defvalue)
{
	imgx = 0;
	img = 0;
}

TColorEdit::~TColorEdit()
{
	if(img) delete img;
}

void TColorEdit::Init()
{
  uint max_value_w = 20;
  uint margin = 10;

  /* Boutons */
  if(m_plus) delete m_plus;
  if(m_minus) delete m_minus;

  m_plus = new TButton (x+w-5,y,5,10);
  m_minus = new TButton (x+w-max_value_w-5-2*margin,y,5,10);

  MyComponent(m_plus);
  MyComponent(m_minus);

  /* Images */
  /* Pas besoin de delete, ECImage le fait */
  m_plus->SetImage (new ECSprite(Resources::UpButton(), Window()));
  m_minus->SetImage (new ECSprite(Resources::DownButton(), Window()));

  /* Label */
  if(txt_label) delete txt_label;

  imgx = m_minus->X() + m_minus->Width() + 5;

  txt_label = new TLabel(x, y, label, color, font);

  img = new ECImage(SDL_CreateRGBSurface( SDL_SWSURFACE, 20, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));

  SetValue(value, true);
}

bool TColorEdit::SetValue(int _value, bool first)
{
  _value = BorneLong(_value, min, max);

  if(_value == value && !first) return false;
  value = _value;

  SDL_Color *color = color_eq[value];

  SDL_Rect r_back = {0,0,img->Img->w,img->Img->h};

  if(value)
    SDL_FillRect( img->Img, &r_back, SDL_MapRGB( img->Img->format,color->r, color->g, color->b));
  else
    SDL_FillRect( img->Img, &r_back, SDL_MapRGBA( img->Img->format, 255,255,255, 0));

  return true;
}

void TColorEdit::Draw (int mouse_x, int mouse_y)
{
	if(txt_label)
		txt_label->Draw(mouse_x, mouse_y);
	
	if(img)
	{
		SDL_Rect r_back = {imgx,y,img->Img->w,img->Img->h};
		SDL_BlitSurface( img->Img, NULL, Window(), &r_back);
	}

	if(enabled)
	{
		if(m_minus)
			m_minus->Draw (mouse_x, mouse_y);
		if(m_plus)
			m_plus->Draw (mouse_x, mouse_y);
	}
}
