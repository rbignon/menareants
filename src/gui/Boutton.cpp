/* src/gui/Boutton.cpp - Boutton
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

#include "Boutton.h"
#include "tools/Images.h"

Button::Button ()
{
   image = NULL;
   SetPos(0,0);
   SetSize(0,0);
}

//-----------------------------------------------------------------------------

Button::Button (unsigned int x, unsigned int y, unsigned int w, unsigned int h)
  : m_x(x), m_width(w), m_y(y), m_height(h)
{
	image = NULL;
}

//-----------------------------------------------------------------------------

Button::~Button()
{
	if(image) delete image;
}

void Button::SetPos (unsigned int x, unsigned int y)
{
  m_x  = x;
  m_y  = y;
}

//-----------------------------------------------------------------------------

void Button::SetSize (unsigned int w, unsigned int h)
{
  m_width = w;
  m_height = h;
}

//-----------------------------------------------------------------------------

bool Button::Test (unsigned int souris_x, unsigned int souris_y)
{
  return ((m_x <= souris_x) && (souris_x <= m_x+m_width)
	  && (m_y <= souris_y) && (souris_y <= m_y+m_height));
}

void Button::Draw (unsigned int souris_x, unsigned int souris_y)
{
   DrawImage (souris_x, souris_y);
}

//-----------------------------------------------------------------------------

void Button::DrawImage (unsigned int souris_x, unsigned int souris_y)
{
  unsigned int frame = Test(souris_x,souris_y) ? 1 : 0;

  image->setFrame(frame);
  image->set(m_x, m_y);
  image->draw();

}

void Button::SetImage (ECSprite *_image)
{
  if(image) delete image;
  image = _image;
  SetSize (image->GetWidth(), image->GetHeight());
}

unsigned int Button::GetX() const { return m_x; }
unsigned int Button::GetY() const { return m_y; }
unsigned int Button::GetWidth() const { return m_width; }
unsigned int Button::GetHeight() const { return m_height; }
