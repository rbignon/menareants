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
#include "Sound.h"
#include <SDL_gfxPrimitives.h>

/********************************************************************************************
 *                                ECMcDo                                                    *
 ********************************************************************************************/

void ECMcDo::RecvData(ECData data)
{
	switch(data.type)
	{
		case DATA_INVESTED:
		{
			invested = static_cast<ECEntity::e_type>(StrToTyp<uint>(data.data));
			SetMyStep(0);
			SetRestStep(0);
			break;
		}
		case DATA_EXOWNER:
		{
			ex_owner = data.data;
			break;
		}
	}
}

std::string ECMcDo::SpecialInfo()
{
	if(!invested)
		return "";
	else if(ex_owner == Owner()->GetNick())
		return "Consommations de McGerbale gratuites pour " + ex_owner + " !";
	else
		return "1000 $ consommations payés chaque tour à " + ex_owner;
}

bool ECMcDo::CanCreate(const ECBEntity* entity)
{
	if(!Deployed() || !invested) return false;

	return EntityList.Get(invested)->CanCreate(entity);
}

/********************************************************************************************
 *                                ECTourist                                                 *
 ********************************************************************************************/

void ECTourist::ChangeCase(ECBCase* newcase)
{
	/* Le touriste ne permet pas aux cases par lesquelles il est passé de se remettre en Showed == 0.
	 * Elles sont donc visibles en permanence. Pour cela, il va incrémenter deux fois Showed quand il arrive,
	 * comme ça meme quand il partira ça décrémentera qu'une fois et la valeur sera au minimum de 1.
	 */
	ECEntity::ChangeCase(newcase);
	SetShowedCases(true);
}

/********************************************************************************************
 *                                ECMissiLauncher                                           *
 ********************************************************************************************/

bool ECMissiLauncher::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			if(!missile.Missile() && c != Case()) // c'est pas une attaque sur moi donc je tire
			{
				SetImage(GetSprite(I_Reployed));
				Image()->SetAnim(false);
			}
			return missile.AttaqFirst(c, me);
		default:
			return ECUnit::BeforeEvent(entities,c, me);
	}
}

bool ECMissiLauncher::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			return missile.AttaqSecond(c, me);
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
			return true;
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
		images[i]->ChangeColor(white_color, color_eq[Owner()->Color()]);

	if(i == I_Down) SetImage(images[i]);
}

void ECUnit::RefreshColor(Color last)
{
	for(ImgList::iterator it = images.begin(); it != images.end(); ++it)
		it->second->ChangeColor(last, Owner() ? color_eq[Owner()->Color()] : white_color);
}

ECUnit::~ECUnit()
{
	for(ImgList::iterator it = images.begin(); it != images.end(); ++it)
		delete it->second;
}

bool ECUnit::BeforeEvent(const std::vector<ECEntity*>&, ECase*, EC_Client*)
{
	if(event_type & ARM_MOVE)
	{
		if(!move.Empty())
			SetImage(images[(imgs_t)move.First()]);
		SetAnim(true);
	}
	return true;
}

bool ECUnit::MoveEffect(const std::vector<ECEntity*>& entities)
{
	ECMap* map = dynamic_cast<ECMap*>(Case()->Map());
	if(move.Empty())
		return true;

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
		case ARM_DEPLOY:
		{
			if(Deployed())
			{
				ECSpriteBase* sprite = images[I_Deployed];
				if(!sprite) break;
				SetImage(sprite);
				if(Case()->Showed() <= 0)
					Image()->SetFrame(Image()->NbFrames()-1);
				else
				{
					Image()->SetRepeat(false);
					Image()->SetAnim(true);
					WAIT_EVENT_T(!Image()->Anim(), i, 10);
					Image()->SetRepeat(true);
					Image()->SetAnim(false);
				}
			}
			/* C'est le reploiement alors que je viens de tirer, donc il n'y a plus de missile sur la rampe */
			else if(dynamic_cast<EChannel*>(Case()->Map()->Channel())->CurrentEvent() & ARM_ATTAQ)
			{
				if(Case()->Showed() > 0)
				{
					ECSpriteBase* sprite = images[I_Reployed];
					if(!sprite) break;
					SetImage(sprite);
					Image()->SetRepeat(false);
					Image()->SetAnim(true);
					WAIT_EVENT_T(!Image()->Anim(), i, 10);
					Image()->SetRepeat(true);
					Image()->SetAnim(false);
					SDL_Delay(20);
				}
				SetImage(images[I_Right]);
			}
			/* C'est un reploiement sans tirer, donc on utilise l'image du déploiement avec missile, mais dans l'ordre
			 * inverse */
			else
			{
				if(Case()->Showed() > 0)
				{
					ECSpriteBase* sprite = images[I_Deployed];
					if(!sprite) break;
					SetImage(sprite);
					Image()->SetOrder(false);
					Image()->SetFrame(Image()->NbFrames()-1);
					Image()->SetRepeat(false);
					Image()->SetAnim(true);
					WAIT_EVENT_T(!Image()->Anim(), i, 10);
					Image()->SetRepeat(true);
					Image()->SetAnim(false);
					Image()->SetOrder(true);
					SDL_Delay(20);
				}
				SetImage(images[I_Right]);
			}
			break;
		}
		default:
			if(event_type & ARM_MOVE || event_type & ARM_ATTAQ)
				return MoveEffect(entities);
			break;
		
	}
	return true;
}

bool ECUnit::AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*)
{
	if(event_type & ARM_ATTAQ)
	{
		if(Case()->Showed() > 0 && images[I_Attaq])
		{
			if(!AttaqImg())
			{
				SetAttaqImg(images[I_Attaq], Image()->X(), Image()->Y());
				AttaqImg()->SetRepeat(false);
				AttaqImg()->SetAnim(true);
				Resources::SoundMitraillette()->Play(true);
				return false;
			}
			if(AttaqImg()->Anim())
			{
				SDL_Delay(20);
				return false;
			}
	
			SetAttaqImg(0,0,0);
			Resources::SoundMitraillette()->Stop();
		}
	}
	if(event_type & ARM_MOVE)
			SetAnim(false);
	return true;
}
