/* src/Units.cpp - Code of units
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

#include "Units.h"
#include "gui/ShowMap.h"
#include "gui/ColorEdit.h"
#include "Channels.h"
#include "Sockets.h"
#include <SDL_gfxPrimitives.h>

/********************************************************************************************
 *                                ECMissiLauncher                                           *
 ********************************************************************************************/

void ECMissiLauncher::Draw()
{
	ECEntity::Draw();
	if(missile)
		missile->draw();
#ifndef WIN32
	if(Selected())
		circleColor(Image()->Window(), Image()->X() + Image()->GetWidth()/2, Image()->Y() + Image()->GetHeight()/2,
		            MISSILAUNCHER_PORTY * CASE_WIDTH,
		            SDL_MapRGB(Image()->Window()->format, blue_color.r, blue_color.g, blue_color.b));
#endif
}

void ECMissiLauncher::SetMissile(ECSpriteBase* c)
{
	if(missile)
		MyFree(missile);
	if(!c) return;

	missile = new ECSprite(c, Image()->Window());
	missile->SetAnim(true);
}

bool ECMissiLauncher::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		{
			if(c == Case() || Case()->Showed() <= 0) return true;

			if(!missile)
			{
				me->LockScreen();
				dynamic_cast<ECMap*>(acase->Map())->ShowMap()->CenterTo(this);
				SetImage(GetSprite(I_Reployed));
				Image()->SetAnim(false);
				SetMissile(Resources::MissiLauncher_Missile_Up());
				missile->set(Image()->X(), Image()->Y());
				me->UnlockScreen();
				return false;
			}
			missile->set(Image()->X(), missile->Y() - MISSILAUNCHER_MISSILE_STEP);
			if(missile->Y() + missile->GetHeight() <= 0)
			{
				me->LockScreen();
				dynamic_cast<ECMap*>(acase->Map())->ShowMap()->CenterTo(c);
				SetMissile(Resources::MissiLauncher_Missile_Down());
				missile->set(c->Image()->X(), 0 - missile->GetHeight());
				me->UnlockScreen();
				return true;
			}
			SDL_Delay(20);
			return false;
			break;
		}
		default:
			return ECUnit::BeforeEvent(entities,c, me);
	}
}

bool ECMissiLauncher::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_ATTAQ|ARM_MOVE:
		{
			if(c == Case() || !missile || c->Showed() <= 0) return true;

			missile->set(missile->X(), missile->Y() + MISSILAUNCHER_MISSILE_STEP);
			if(missile->Y() >= c->Image()->Y())
			{
				me->LockScreen();
				MyFree(missile);
				me->UnlockScreen();
				return true;
			}
			SDL_Delay(20);
			return false;

			break;
		}
		default:
			return ECUnit::MakeEvent(entities,c, me);
	}
	return true;
}

bool ECMissiLauncher::AfterEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		{
			if(c == Case()) return true;

			if(c->Flags() & (C_TERRE))
				c->Image()->SetFrame(1);

			SDL_Delay(300);
   return true;

			break;
		}
		default:
			return ECUnit::AfterEvent(entities,c, me);
	}
	return true;
}

/********************************************************************************************
 *                                ECUnit                                                     *
 ********************************************************************************************/

void ECUnit::PutImage(imgs_t i, ECSpriteBase* b)
{
	images.insert(ImgList::value_type(i, new ECSpriteBase(b->path.c_str())));

	if(Owner() && Owner()->Color())
		images[i]->ChangeColor(white_color, *color_eq[Owner()->Color()]);

	if(i == I_Down) SetImage(images[i]);
}

void ECUnit::RefreshColor(SDL_Color last)
{
	for(ImgList::iterator it = images.begin(); it != images.end(); ++it)
		it->second->ChangeColor(last, Owner() ? *color_eq[Owner()->Color()] : white_color);
}

ECUnit::~ECUnit()
{
	for(ImgList::iterator it = images.begin(); it != images.end(); ++it)
		delete it->second;
}

bool ECUnit::BeforeEvent(const std::vector<ECEntity*>&, ECase*, EC_Client*)
{
	switch(event_type)
	{
		case ARM_MOVE:
		case ARM_ATTAQ|ARM_MOVE:
			if(!move.Empty())
				SetImage(images[(imgs_t)move.First()]);
			SetAnim(true);
			break;
		default: break;
	}
	return true;
}

bool ECUnit::MoveEffect(const std::vector<ECEntity*>& entities)
{
	ECMap* map = dynamic_cast<ECMap*>(Case()->Map());
	if(move.Empty())
	{
		if(event_type & ARM_ATTAQ)
			SetImage(images[I_Attaq]);
		return true;
	}

	ECMove::E_Move m = move.First();
	switch(m)
	{
		case ECMove::Right: ImageSetXY(Image()->X() + visual_step, Image()->Y()); break;
		case ECMove::Left:  ImageSetXY(Image()->X() - visual_step, Image()->Y()); break;
		case ECMove::Down:  ImageSetXY(Image()->X(), Image()->Y() + visual_step); break;
		case ECMove::Up:    ImageSetXY(Image()->X(), Image()->Y() - visual_step); break;
	}

	SDL_Delay(20/entities.size());
	bool changed_case = false;
	switch(m)
	{
		case ECMove::Right:
			if(map->ShowMap()->X() + (CASE_WIDTH * int(acase->X()+1)) <= Image()->X())
				ChangeCase(acase->MoveRight()), move.RemoveFirst(), changed_case = true;
			break;
		case ECMove::Left:
			if(map->ShowMap()->X() + (CASE_WIDTH * int(acase->X()-1)) >= Image()->X())
				ChangeCase(acase->MoveLeft()), move.RemoveFirst(), changed_case = true;
			break;
		case ECMove::Down:
			if(map->ShowMap()->Y() + (CASE_HEIGHT * int(acase->Y()+1)) <= Image()->Y())
				ChangeCase(acase->MoveDown()), move.RemoveFirst(), changed_case = true;
			break;
		case ECMove::Up:
			if(map->ShowMap()->Y() + (CASE_HEIGHT * int(acase->Y()-1)) >= Image()->Y())
				ChangeCase(acase->MoveUp()), move.RemoveFirst(), changed_case = true;
			break;
	}
	if(changed_case && entities.size() == 1 && Case()->Showed() > 0 &&
	   dynamic_cast<ECMap*>(acase->Map())->ShowMap())
		dynamic_cast<ECMap*>(acase->Map())->ShowMap()->CenterTo(this);

	if(!move.Empty() && m != move.First())
		SetImage(images[(imgs_t)move.First()]);
	return false;
}

bool ECUnit::MakeEvent(const std::vector<ECEntity*>& entities, ECase*, EC_Client*)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
		case ARM_ATTAQ|ARM_MOVE:
		case ARM_UNION:
			return MoveEffect(entities);
		case ARM_DEPLOY:
		{
			if(Case()->Showed() <= 0) return true;
			if(Deployed())
			{
				ECSpriteBase* sprite = images[I_Deployed];
				if(!sprite) break;
				SetImage(sprite);
				Image()->SetRepeat(false);
				Image()->SetAnim(true);
				WAIT_EVENT_T(!Image()->Anim(), i, 10);
			}
			else
			{
				ECSpriteBase* sprite = images[I_Reployed];
				if(!sprite) break;
				SetImage(sprite);
				Image()->SetRepeat(false);
				Image()->SetAnim(true);
				WAIT_EVENT_T(!Image()->Anim(), i, 10);
				Image()->SetRepeat(true);
				SDL_Delay(20);
				SetImage(images[I_Right]);
			}
			break;
		}
		default: break;
	}
	return true;
}

bool ECUnit::AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_ATTAQ|ARM_MOVE:
			SDL_Delay(500);
			SetImage(images[I_Down]);
			if(c->Flags() & (C_TERRE))
				c->Image()->SetFrame(1);
			// pas de break ici
		case ARM_MOVE:
			SetAnim(false);
			break;
		default: break;
	}
	return true;
}
