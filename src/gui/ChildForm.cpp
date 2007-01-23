/* src/gui/ChildForm.cpp - This is a form include in another form
 *
 * Copyright (C) 2005,2007 Romain Bignon  <Progs@headfucking.net>
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

#include "ChildForm.h"

TChildForm::TChildForm(int _x, int _y, uint _w, uint _h)
	: TComponent(_x, _y, _w, _h), background(0), focus_order(true)
{
	composants.clear();
}

void TChildForm::SetBackground(ECImage *image)
{
	background = image;
}

void TChildForm::DelFocus()
{
	TComponent::DelFocus();
	for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
		(*it)->DelFocus();
}

void TChildForm::PressKey(SDL_keysym k)
{
	for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
		(*it)->PressKey(k);
}

bool TChildForm::Clic(const Point2i& mouse, int _button)
{
	if(!Test(mouse,_button)) return false;

	TComponent* clicked = 0;
	/* Va dans l'ordre inverse */
	for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
		if((*it)->Visible() && !clicked && (*it)->Clic(mouse, _button))
		{
			(*it)->SetFocus();
			if((*it)->OnClick())
				(*(*it)->OnClick()) (*it, (*it)->OnClickParam());
			if((*it)->OnClickPos())
				(*(*it)->OnClickPos()) (*it, mouse);
			clicked = *it;
			break;
		}

	for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
		if(!(*it)->ForceFocus() && *it != clicked)
			(*it)->DelFocus();
	return true;
}

void TChildForm::Draw(const Point2i& mouse)
{
	if(background)
	{
		SDL_Rect r_back = {X(),Y(),Width(),Height()};
		Window()->Blit(background, &r_back);
	}

	bool first = focus_order ? true : false, put_hint = false;
	while(1)
	{
		for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
			// Affiche seulement à la fin les composants sélectionnés
			if((*it)->Visible() && (!focus_order || (*it)->Focused() == (first ? false : true)) &&
			   (WantRedraw() || (*it)->WantRedraw() || lastmpos != mouse && (*it)->Mouse(mouse) || (*it)->Mouse(lastmpos) && !(*it)->Mouse(mouse)))
			{
				if(background)
					Window()->Blit(background, **it, (*it)->GetPosition());
				if((*it)->OnMouseOn() && (*it)->Mouse(mouse))
					(*(*it)->OnMouseOn()) (*it, (*it)->OnMouseOnParam());
				(*it)->Draw(mouse);
				if(!put_hint && (*it)->Visible() && (*it)->HaveHint() && (*it)->Mouse(mouse))
				{
					SetHint((*it)->Hint());
					put_hint = true;
				}
				(*it)->SetWantRedraw(false);
			}
		if(first) first = false;
		else break;
	}
	lastmpos = mouse;
	if(!put_hint)
		SetHint("");
}

void TChildForm::Clear()
{
	for(std::vector<TComponent*>::reverse_iterator it = composants.rbegin(); it != composants.rend(); ++it)
		delete *it;
	composants.clear();
}

void TChildForm::SetXY(int _x, int _y)
{
	for(std::vector<TComponent*>::iterator it = composants.begin(); it != composants.end(); ++it)
		(*it)->SetXY(((*it)->X() - X()) + _x, ((*it)->Y() - Y()) + _y);
	TComponent::SetXY(_x, _y);
}
