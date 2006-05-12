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

/********************************************************************************************
 *                                ECMissiLauncher                                           *
 ********************************************************************************************/

bool ECMissiLauncher::MakeEvent(const std::vector<ECEntity*>& entities)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
		case ARM_ATTAQ|ARM_MOVE:
		case ARM_UNION:
			return MoveEffect(entities);
		case ARM_DEPLOY:
			if(!Owner() || !dynamic_cast<ECPlayer*>(Owner())->IsMe())
				SetDeployed(!Deployed());
			break;
		default: break;
	}
	return true;
}

/********************************************************************************************
 *                                ECUnit                                                     *
 ********************************************************************************************/

void ECUnit::PutImage(imgs_t i, ECSpriteBase* b)
{
	images.insert(ImgList::value_type(i, new ECSpriteBase(b->path.c_str())));

	if(Owner())
		images[i]->ChangeColor(white_color, *color_eq[Owner()->Color()]);

	if(i == I_Down) SetImage(images[i]);
}

ECUnit::~ECUnit()
{
	for(ImgList::iterator it = images.begin(); it != images.end(); ++it)
		delete it->second;
}

bool ECUnit::BeforeEvent(const std::vector<ECEntity*>&)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
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
		case ECMove::Right: image->set(image->X() + visual_step, image->Y()); break;
		case ECMove::Left:  image->set(image->X() - visual_step, image->Y()); break;
		case ECMove::Down:  image->set(image->X(), image->Y() + visual_step); break;
		case ECMove::Up:    image->set(image->X(), image->Y() - visual_step); break;
	}
	SDL_Delay(20/entities.size());
	switch(m)
	{
		case ECMove::Right:
			if(map->ShowMap()->X() + (CASE_WIDTH * int(acase->X()+1)) <= image->X())
				ChangeCase(acase->MoveRight()), move.RemoveFirst();
			break;
		case ECMove::Left:
			if(map->ShowMap()->X() + (CASE_WIDTH * int(acase->X()-1)) >= image->X())
				ChangeCase(acase->MoveLeft()), move.RemoveFirst();
			break;
		case ECMove::Down:
			if(map->ShowMap()->Y() + (CASE_HEIGHT * int(acase->Y()+1)) <= image->Y())
				ChangeCase(acase->MoveDown()), move.RemoveFirst();
			break;
		case ECMove::Up:
			if(map->ShowMap()->Y() + (CASE_HEIGHT * int(acase->Y()-1)) >= image->Y())
				ChangeCase(acase->MoveUp()), move.RemoveFirst();
			break;
	}
	if(!move.Empty() && m != move.First())
		SetImage(images[(imgs_t)move.First()]);
	return false;
}

bool ECUnit::MakeEvent(const std::vector<ECEntity*>& entities)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
		case ARM_ATTAQ|ARM_MOVE:
		case ARM_UNION:
			return MoveEffect(entities);
		default: break;
	}
	return true;
}

bool ECUnit::AfterEvent(const std::vector<ECEntity*>&)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_ATTAQ|ARM_MOVE:
			SDL_Delay(500);
			SetImage(images[I_Down]);
		case ARM_MOVE:
			SetAnim(false);
			break;
		default: break;
	}
	return true;
}
