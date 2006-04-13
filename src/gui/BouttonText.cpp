/* src/gui/BouttonText.cpp - Boutton Text
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

#include "BouttonText.h"

TButtonText::TButtonText() : TButton(), font(0)
{
}

//-----------------------------------------------------------------------------

TButtonText::TButtonText (int _x, int _y, unsigned int _w, unsigned int _h, const std::string &text, Font *f)
  : TButton(_x, _y, _w, _h), surf(0)
{
  font = f;
  SetText(text);
}

//-----------------------------------------------------------------------------

void TButtonText::SetFont (Font *font)
{
  this->font = font;
}

//-----------------------------------------------------------------------------

void TButtonText::SetText(const std::string &text)
{
  if (surf != NULL)
  {
    SDL_FreeSurface(surf);
  }
  assert (font != NULL);
  m_text = text;
  surf = TTF_RenderText_Blended(&(font->GetTTF()), text.c_str(), Enabled() ? white_color : gray_color);
}

void TButtonText::SetEnabled(bool b)
{
	TComponent::SetEnabled(b);
	SetText(m_text);
}

//-----------------------------------------------------------------------------

void TButtonText::Draw (int souris_x, int souris_y)
{
	if(image)
		DrawImage (souris_x, souris_y);

	if(!surf) return;

	SDL_Rect dst_rect;
	dst_rect.x = X()+Width()/2-surf->w/2;
	dst_rect.y = Y()+Height()/2-surf->h/2;
	dst_rect.w = surf->w;
	dst_rect.h = surf->h;

	SDL_BlitSurface(surf,NULL,Window(), &dst_rect);
}

//-----------------------------------------------------------------------------

std::string TButtonText::GetText() const
{
  return m_text;
}
