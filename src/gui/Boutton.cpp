/* src/gui/Boutton.cpp - Boutton
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
 *
 * $Id$
 */

#include "Boutton.h"
#include "tools/Images.h"

TButton::TButton ()
{
   image = NULL;
   SetXY(0,0);
   w = 0;
   h = 0;
}

//-----------------------------------------------------------------------------

TButton::TButton (unsigned int _x, unsigned int _y, unsigned int _w, unsigned int _h)
  : TComponent(_x, _y, _w, _h)
{
	image = NULL;
	enabled = true;
}

//-----------------------------------------------------------------------------

TButton::~TButton()
{
	if(image) delete image;
}

//-----------------------------------------------------------------------------

bool TButton::Test (unsigned int souris_x, unsigned int souris_y) const
{
  return (((x <= souris_x) && (souris_x <= x+w)
	  && (y <= souris_y) && (souris_y <= y+h)) && enabled);
}

void TButton::Draw (unsigned int souris_x, unsigned int souris_y)
{
   DrawImage (souris_x, souris_y);
}

//-----------------------------------------------------------------------------

void TButton::DrawImage (unsigned int souris_x, unsigned int souris_y)
{
  if(!image) return;

  unsigned int frame = Test(souris_x,souris_y) ? 1 : 0;

  image->setFrame(frame);
  image->set(x, y);
  image->draw();

}

void TButton::SetImage (ECSprite *_image)
{
  if(image) delete image;
  image = _image;
  SetWidth(image->GetWidth());
  SetHeight(image->GetHeight());
}
