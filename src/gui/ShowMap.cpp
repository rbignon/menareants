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
#include "tools/Video.h"
#include "Defines.h"
#include "Resources.h"
#include "Channels.h"
#include "Debug.h"
#include "tools/Font.h"
#include <SDL_gfxPrimitives.h>

void TMap::Init()
{
	SetPosition(x, y, true);
	h = CASE_HEIGHT * map->Height();
	w = CASE_WIDTH  * map->Width();

	SDL_Rect r_back = {0,0,CASE_WIDTH, CASE_HEIGHT};
	brouillard.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_OPENGL, CASE_WIDTH, CASE_HEIGHT,
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
	brouillard.FillRect(r_back, brouillard.MapRGBA(0, 0, 0, 255*5/10));

	SetMustRedraw();
}

void TMap::SetXY(int _x, int _y)
{
	SetPosition(_x, _y);
}

void TMap::MoveTo(int _x, int _y)
{
	SetPosition(- _x, - _y);
}

void TMap::CenterTo(int _x, int _y)
{
	MoveTo(_x - SCREEN_WIDTH/2, _y - SCREEN_HEIGHT/2);
}

void TMap::ScrollTo(int _x, int _y)
{
	/// \todo Implementation of this function
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
		if(*casi)
			dynamic_cast<ECase*>(*casi)->Image()->set(x+(CASE_WIDTH  * (*casi)->X()),
		                                              y+(CASE_HEIGHT * (*casi)->Y()));

	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
		if((*enti)->Case())
			dynamic_cast<ECEntity*>(*enti)->Image()->set(x+(CASE_WIDTH  * (*enti)->Case()->X()),
		                                                 y+(CASE_HEIGHT * (*enti)->Case()->Y()));

	SetMustRedraw();
}

ECEntity* TMap::TestEntity(int mouse_x, int mouse_y)
{
	if(!Focused()) return 0;

	ECase* c = TestCase(mouse_x, mouse_y);

	if(c->Entities()->empty()) return 0;

	ECBEntity* e = c->Entities()->First();

	c->Entities()->Remove(e);
	c->Entities()->Add(e);

	return dynamic_cast<ECEntity*>(e);
#if 0
	std::vector<ECBEntity*> entities = map->Entities()->List();

	/* On porte la priorité sur la selection des entités plutot que des batiments */
	ECEntity* et = 0;
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
		if(dynamic_cast<ECEntity*>(*enti)->Test(mouse_x, mouse_y))
		{
			et = dynamic_cast<ECEntity*>(*enti);
			if(!(*enti)->IsBuilding())
				return et;
		}
	return et;
#endif
}

ECase* TMap::TestCase(int mouse_x, int mouse_y)
{
	if(!Focused()) return 0;

	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		if(*casi && dynamic_cast<ECase*>(*casi)->Test(mouse_x, mouse_y))
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
		if(_x >=0 && _x < 15)
			xx += 15;
		if(_y >= 0 && _y < 15)
			yy += 15;
		if(_x > int(SCREEN_WIDTH-15) && _x <= int(SCREEN_WIDTH))
			xx -= 15;
		if(_y > int(SCREEN_HEIGHT-15) && _y <= int(SCREEN_HEIGHT))
			yy -= 15;
	
		if(xx != x || yy != y)
			SetXY(xx, yy);
	}

	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
	{
		ECase* c = dynamic_cast<ECase*>(*casi);
		if(c && (MustRedraw() || c->MustRedraw()))
		{
			if(HaveBrouillard() && c->Showed() < 0)
			{
				SDL_Rect r_back = {c->Image()->X(),c->Image()->Y(),CASE_WIDTH,CASE_HEIGHT};
				Window()->FillRect(r_back, 0);
				continue;
			}
			c->Draw();
			if(CreateEntity() && c->Test(_x, _y))
					((!CreateEntity()->Owner() || CreateEntity()->CanBeCreated(c)) ? Resources::GoodHashure()
					                                                               : Resources::BadHashure())
					                     ->Draw(c->Image()->X(), c->Image()->Y());
			if(HaveBrouillard() && c->Showed() == 0)
			{
				SDL_Rect r_back = {c->Image()->X(),c->Image()->Y(),brouillard.GetWidth(),brouillard.GetHeight()};
				Window()->Blit(brouillard, &r_back);
				//SDL_Rect r_back = {c->Image()->X(),c->Image()->Y(),CASE_WIDTH,CASE_HEIGHT};
				//SDL_FillRect( Window(), &r_back, SDL_MapRGBA( Window()->format,0, 0, 0, 255*5/10));
			}
			if(schema)
				Resources::Case()->Draw(c->Image()->X(), c->Image()->Y());
		}
	}

	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end();)
		if(!(*enti))
		{
			map->RemoveAnEntity(*enti);
			enti = entities.erase(enti);
		}
		else
		{
			dynamic_cast<ECEntity*>(*enti)->Draw();
			++enti;
		}
#if 0
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		if(dynamic_cast<ECase*>(*casi)->Showed() == 0)
		{
			SDL_Rect dest;
			dest.x = dynamic_cast<ECase*>(*casi)->Image()->X();
			dest.y = dynamic_cast<ECase*>(*casi)->Image()->Y();
			SDL_BlitSurface(brouillard, NULL, Window(), &dest);
		}
#endif

	if(map->Channel())
		for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
	{
		ECEntity* entity = dynamic_cast<ECEntity*>(*enti);
		if(!entity->Move()->Empty())
		{
			if(map->Channel()->State() == EChannel::ANIMING)
			{

			}
			else if(map->Channel()->State() == EChannel::PLAYING)
			{
				ECase* c = dynamic_cast<ECase*>((*enti)->Case());
				if(c != entity->Move()->FirstCase())
					FDebug(W_WARNING|W_SEND, "La case de l'entité et le départ du mouvement ne sont pas identiques");

				ECMove::Vector moves = entity->Move()->Moves();
				ECase* next_c = 0;
				int last_move = -1;
				for(ECMove::Vector::iterator move = moves.begin(); move != moves.end(); ++move)
				{
					switch(*move)
					{
						case ECMove::Up: next_c = dynamic_cast<ECase*>(c->MoveUp()); break;
						case ECMove::Down: next_c = dynamic_cast<ECase*>(c->MoveDown()); break;
						case ECMove::Left: next_c = dynamic_cast<ECase*>(c->MoveLeft()); break;
						case ECMove::Right: next_c = dynamic_cast<ECase*>(c->MoveRight()); break;
					}
					if(c->Y() == next_c->Y())
					{
						switch(last_move)
						{
							case ECMove::Up:
								(c->X() < next_c->X() ? Resources::FlecheGaucheBas()
								                      : Resources::FlecheDroiteBas())
								                    ->Draw(c->Image()->X(), c->Image()->Y());
								break;
							case ECMove::Down:
								(c->X() < next_c->X() ? Resources::FlecheGaucheHaut()
								                      : Resources::FlecheDroiteHaut())
								                    ->Draw(c->Image()->X(), c->Image()->Y());
								break;
							case ECMove::Left:
							case ECMove::Right:
							default:
								Resources::FlecheHoriz()->Draw(c->Image()->X(), c->Image()->Y());
								break;
						}
					}
					if(c->X() == next_c->X())
					{
						switch(last_move)
						{
							case ECMove::Left:
								(c->Y() < next_c->Y() ? Resources::FlecheGaucheBas()
								                      : Resources::FlecheGaucheHaut())
								                    ->Draw(c->Image()->X(), c->Image()->Y());
								break;
							case ECMove::Right:
								(c->Y() < next_c->Y() ? Resources::FlecheDroiteBas()
								                      : Resources::FlecheDroiteHaut())
								                    ->Draw(c->Image()->X(), c->Image()->Y());
								break;
							case ECMove::Up:
							case ECMove::Down:
							default:
								Resources::FlecheVert()->Draw(c->Image()->X(), c->Image()->Y());
								break;
						}
					}
					last_move = *move;
					c = next_c;
				}
				switch(last_move)
				{
					case ECMove::Right:
						(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqDroite()
						         : Resources::FlecheVersDroite())->Draw(c->Image()->X(), c->Image()->Y());
						break;
					case ECMove::Left:
						(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqGauche()
						         : Resources::FlecheVersGauche())->Draw(c->Image()->X(), c->Image()->Y());
						break;
					case ECMove::Up:
						(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqHaut()
						         : Resources::FlecheVersHaut())->Draw(c->Image()->X(), c->Image()->Y());
						break;
					case ECMove::Down:
						(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqBas()
						         : Resources::FlecheVersBas())->Draw(c->Image()->X(), c->Image()->Y());
						break;
				}

			}
		}
	}
	SetMustRedraw(false);
}
