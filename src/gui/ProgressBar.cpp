/* src/gui/ProgressBar.cpp - ProgressBar component
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
 * $Id: ProgressBar.cpp 766 2006-06-17 11:41:58Z progs $
 */

#include <SDL.h>
#include "ProgressBar.h"
#include "tools/Maths.h"
#include "tools/Video.h"

TProgressBar::TProgressBar(int x, int y, uint w, uint h)
	: TComponent(x, y, w, h), border_color(0, 0, 0, 255), value_color(255,255,255,255), background_color(100,100,100,255),
	  val(0), min(0), max(0), m_use_ref_val(false), m_ref_val(0), val_barre(0)
{
   border_color.SetColor(0, 0, 0, 255);
   value_color.SetColor(255, 255, 255, 255);
   background_color.SetColor(100, 100 ,100, 255);
   val = min = max = 0;
   m_use_ref_val = false;
   ChangeSize();
}

void TProgressBar::SetBorderColor(Color color)
{
   border_color = color;
}

void TProgressBar::SetBackgroundColor(Color color)
{
   background_color = color;
}

void TProgressBar::SetValueColor(Color color)
{
   value_color = color;
}

void TProgressBar::SetWidth(uint w)
{
	TComponent::SetWidth(w);
	ChangeSize();
}

void TProgressBar::SetHeight(uint h)
{
	TComponent::SetHeight(h);
	ChangeSize();
}

void TProgressBar::ChangeSize()
{
  image.NewSurface(Width(), Height(), SDL_SWSURFACE|SDL_SRCALPHA, true);
}

void TProgressBar::InitVal (long pval, long pmin, long pmax)
{
  assert (pmin != pmax);
  assert (pmin < pmax);
  val = pval;
  min = pmin;
  max = pmax;
  val_barre = CalculeValBarre(val);
}

void TProgressBar::SetValue (long pval)
{
  val = CalculeVal(pval);
  val_barre = CalculeValBarre(val);
}

uint TProgressBar::CalculeVal (long val) const
{
  return BorneLong(val, min, max); 
}

uint TProgressBar::CalculeValBarre (long val) const
{
  return ( CalculeVal(val) -min)*(Width()-2)/(max-min);
}

void TProgressBar::Draw(int souris_x, int souris_y)
{
	int left, right;
	
	// Bordure
	image.Fill(border_color);
	
	// Fond
	SDL_Rect r_back = {1, 1, Width() - 2, Height() - 2};
	image.FillRect(r_back, background_color);
	
	// Valeur
	if (m_use_ref_val)
	{
		int ref = CalculeValBarre (m_ref_val);
		if (val < m_ref_val)
		{
			left = 1+val_barre;
			right = 1+ref;
		}
		else
		{
			left = 1+ref;
			right = 1+val_barre;
		}
	}
	else
	{
		left = 1;
		right = 1+val_barre;
	}
	
	SDL_Rect r_value = {left, 1, right - left, Height() - 2};
	image.FillRect(r_value, value_color);
	
	if (m_use_ref_val)
	{
		int ref = CalculeValBarre (m_ref_val);
		SDL_Rect r_ref = {1 + ref, 1, 1, Height() - 2 };
		image.FillRect(r_ref, border_color);
	}
	
	// Marqueurs
	marqueur_it_const it=marqueur.begin(), fin=marqueur.end();
	for (; it != fin; ++it)
	{
		SDL_Rect r_marq = {1 + it->val, 1, 1, Height() - 2};
		image.FillRect( r_marq, it->color);
	}
	SDL_Rect dst = { X(), Y(), Width(), Height() };
	Video::GetInstance()->Window()->Blit(image, &dst);
}

// Ajoute/supprime un marqueur
TProgressBar::marqueur_it TProgressBar::AddMarqueur (long val, const Color& color)
{
  marqueur_t m;

  m.val = CalculeValBarre (val);
  m.color = color;
  marqueur.push_back (m);

  return --marqueur.end();
}

void TProgressBar::RemoveMarqueur (marqueur_it it)
{
  marqueur.erase (it);
}

void TProgressBar::ResetMarqueur()
{
  marqueur.clear();
}

void TProgressBar::SetReferenceValue (bool use, long value)
{
  m_use_ref_val = use;
  m_ref_val = CalculeVal(value);
}
