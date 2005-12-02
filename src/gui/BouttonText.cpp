/* src/gui/BouttonText.cpp - Boutton Text
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

#include "BouttonText.h"

TButtonText::TButtonText() : TButton()
{
  font = NULL;
}

//-----------------------------------------------------------------------------

TButtonText::TButtonText (unsigned int x, unsigned int y, unsigned int w, unsigned int h, const std::string &text)
  : TButton(x, y, w, h)
{
  m_text = text;
}


//-----------------------------------------------------------------------------

void TButtonText::SetFont (Font *font)
{
  this->font = font;
}

//-----------------------------------------------------------------------------

void TButtonText::SetText(const std::string &text)
{
  m_text = text;
}

//-----------------------------------------------------------------------------

void TButtonText::Draw (unsigned int souris_x, unsigned int souris_y)
{
  if(image)
    DrawImage (souris_x, souris_y);

  assert (font != NULL);

  const int x = GetX()+GetWidth()/2;
  const int y = GetY()+GetHeight()/2;

  font->WriteCenter (x, y, m_text, enabled ? white_color : gray_color);
}

//-----------------------------------------------------------------------------

std::string TButtonText::GetText() const
{
  return m_text;
}
