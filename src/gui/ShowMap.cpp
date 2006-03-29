/* src/gui/ShowMap.cpp - This is a component to show the map
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
 * $Id$
 */

#include "ShowMap.h"
#include "Defines.h"
#include "Resources.h"
#include "Channels.h"

void TMap::Init()
{
	SetPosition(x, y, true);
	h = CASE_HEIGHT * map->Height();
	w = CASE_WIDTH  * map->Width();
}

void TMap::SetXY(int _x, int _y)
{
	SetPosition(_x, _y);
}

void TMap::SetPosition(int _x, int _y, bool force)
{
	if(!map) return;

	if(_x > 0) _x = 0;
	if(Xmin() <= 0 && _x < Xmin()) _x = Xmin();
	if(Xmin() > 0) _x = x;
	if(_y > 0) _y = 0;
	if(Ymin() <= 0 && _y < Ymin()) _y = Ymin();
	if(Ymin() > 0) _y = y;

	if(!force && x == _x && y == _y) return;

	x = _x;
	y = _y;

	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		dynamic_cast<ECase*>(*casi)->Image()->set(x+(CASE_WIDTH  * (*casi)->X()),
		                                          y+(CASE_HEIGHT * (*casi)->Y()));

	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
		dynamic_cast<ECEntity*>(*enti)->Image()->set(x+(CASE_WIDTH  * (*enti)->Case()->X()),
		                                          y+(CASE_HEIGHT * (*enti)->Case()->Y()));
}

ECEntity* TMap::TestEntity(int mouse_x, int mouse_y)
{
	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
		if(dynamic_cast<ECEntity*>(*enti)->Test(mouse_x, mouse_y))
			return dynamic_cast<ECEntity*>(*enti);
	return 0;
}

ECase* TMap::TestCase(int mouse_x, int mouse_y)
{
	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		if(dynamic_cast<ECase*>(*casi)->Test(mouse_x, mouse_y))
			return dynamic_cast<ECase*>(*casi);
	return 0;
}

void TMap::Draw(int _x, int _y)
{
	if(!map) return;

	if(Enabled())
	{
		int xx = x, yy = y;

		/* Changement de position automatique */
		if(_x && _x < 15)
			xx += 4;
		if(_y && _y < 15)
			yy += 4;
		if(_x > int(SCREEN_WIDTH-15) && _x < int(SCREEN_WIDTH))
			xx -= 4;
		if(_y > int(SCREEN_HEIGHT-15) && _y < int(SCREEN_HEIGHT))
			yy -= 4;
	
		if(xx != x || yy != y)
			SetXY(xx, yy);
	}

	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		dynamic_cast<ECase*>(*casi)->Draw();

	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
		dynamic_cast<ECEntity*>(*enti)->Draw();

	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
	{
		ECEntity* entity = dynamic_cast<ECEntity*>(*enti);
		if(entity->NewCase() && entity->Case() != entity->NewCase())
		{
			if(map->Channel()->State() == EChannel::ANIMING)
			{

			}
			else if(map->Channel()->State() == EChannel::PLAYING)
			{
				ECase* c = dynamic_cast<ECase*>((*enti)->Case());
				int mx = entity->NewCase()->X(), my = entity->NewCase()->Y();

				bool first = true;
				while(int(c->X()) != mx)
				{
					if(first) first = false;
					else Resources::FlecheHoriz()->Draw(c->Image()->X(), c->Image()->Y());
					if(int(c->X()) < mx)
						c = dynamic_cast<ECase*>(c->MoveRight());
					else
						c = dynamic_cast<ECase*>(c->MoveLeft());
				}
				if(my == int(c->Y()))
				{
					if(c->X() < entity->Case()->X())
						(entity->EventType() == ARM_ATTAQ ? Resources::FlecheAttaqGauche()
						         : Resources::FlecheVersGauche())->Draw(c->Image()->X(), c->Image()->Y());
					else if(c->X() > entity->Case()->X())
						(entity->EventType() == ARM_ATTAQ ? Resources::FlecheAttaqDroite()
						         : Resources::FlecheVersDroite())->Draw(c->Image()->X(), c->Image()->Y());
				}
				else if(my > int(c->Y()))
				{
					if(entity->Case()->X() > c->X())
						Resources::FlecheGaucheBas()->Draw(c->Image()->X(), c->Image()->Y());
					else if(entity->Case()->X() < c->X())
						Resources::FlecheDroiteBas()->Draw(c->Image()->X(), c->Image()->Y());
				}
				else
				{
					if(entity->Case()->X() > c->X())
						Resources::FlecheGaucheHaut()->Draw(c->Image()->X(), c->Image()->Y());
					else if(entity->Case()->X() < c->X())
						Resources::FlecheDroiteHaut()->Draw(c->Image()->X(), c->Image()->Y());
				}

				first = true;
				while(int(c->Y()) != my)
				{
					if(first) first = false;
					else Resources::FlecheVert()->Draw(c->Image()->X(), c->Image()->Y());
					if(int(c->Y()) < my)
						c = dynamic_cast<ECase*>(c->MoveDown());
					else
						c = dynamic_cast<ECase*>(c->MoveUp());
				}
				if(c->Y() < entity->Case()->Y())
					(entity->EventType() == ARM_ATTAQ ? Resources::FlecheAttaqHaut()
						         : Resources::FlecheVersHaut())->Draw(c->Image()->X(), c->Image()->Y());
				else if(c->Y() > entity->Case()->Y())
					(entity->EventType() == ARM_ATTAQ ? Resources::FlecheAttaqBas()
						         : Resources::FlecheVersBas())->Draw(c->Image()->X(), c->Image()->Y());
			}
		}
	}
}
