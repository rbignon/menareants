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
#include "Sound.h"
#include "tools/effects_wave.h"
#include <SDL_gfxPrimitives.h>

/********************************************************************************************
 *                                ECJouano                                                  *
 ********************************************************************************************/

ECJouano::ECAnim ECJouano::Anim;

void ECJouano::ECAnim::Init()
{
	if(anim) return;
	WaveEffect e;
	ECImage f(Point2i(Resources::CaseTerreDead()->First()->GetWidth(), Resources::CaseTerreDead()->First()->GetHeight()), SDL_SWSURFACE);
	f.Blit(Resources::CaseTerreDead()->First());
	anim = e.Wave3dSurface(f, 9, 900, 7.0);
}

std::string ECJouano::SpecialInfo()
{
	if(!Deployed() && (EventType() & ARM_DEPLOY))
		return _("Is going to fart");

	return "";
}

bool ECJouano::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
	{
		case ARM_ATTAQ:
		{
			if(c != Case() || c->Showed() <= 0) break;

			Case()->SetImage(Anim.anim);
			Case()->Image()->SetAnim(true);
			Case()->Image()->SetRepeat(false);
			Resources::SoundProut()->Play(false);
			while(Case()->Image()->Anim()) SDL_Delay(20);

			SetAttaqImg(Resources::Jouano_Fog(), Image()->X()-CASE_WIDTH*2, Image()->Y()-CASE_HEIGHT*2);
			AttaqImg()->SetRepeat(false);
			AttaqImg()->SetAnim(true);
			SDL_Delay(1000);
			SetAttaqImg(0,0,0);

			if((Case()->Flags() & C_TERRE) && Case()->Image()->SpriteBase() == Resources::CaseTerre())
				Case()->SetImage(Resources::CaseTerreDead());

			return true;
		}
		default:
			return ECUnit::BeforeEvent(entities,c, me);
	}
	return true;
}

bool ECJouano::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
	{
		case ARM_ATTAQ:
		{
			return true;
		}
		default:
			return ECUnit::MakeEvent(entities,c, me);
	}
	return true;
}

bool ECJouano::AfterEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
	{
		case ARM_ATTAQ:
			return true;
		default:
			return ECUnit::AfterEvent(entities,c, me);
	}
	return true;
}

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
			Resources::SoundMcDo()->Play();
			break;
		}
		case DATA_EXOWNER:
		{
			ex_owner = Channel()->GetPlayer(data.data.c_str());
			if(ex_owner && ex_owner->IsMe())
				SetShowedCases(true, true);
			break;
		}
		case DATA_JOUANO:
		{
			restDestroy = StrToTyp<int>(data.data);
			SetImage(Resources::McDo_Jouano());
			break;
		}
	}
}

ECMcDo::~ECMcDo()
{
	if(ex_owner && ex_owner->IsMe())
		SetShowedCases(false, true);
}

std::string ECMcDo::SpecialInfo()
{
	if(!invested || !ex_owner)
		return "";
	else if(restDestroy > 0)
		return StringF(ngettext("Jouano is eating all Big Mac ! He will finish it in %d day",
		                        "Jouano is eating all Big Mac ! He will finish it in %d days", restDestroy),
		                        restDestroy);
	else if(ex_owner == Owner())
		return StringF(_("Free orders of McPuke for %s"), ex_owner->GetNick());
	else if(ex_owner->IsMe())
		return StringF(_("%s paies $2000 for orders to you"), Owner() ? Owner()->GetNick() : "Neutral");
	else if(!Owner())
		return StringF(_("Neutral paies $2000 for orders to %s"), ex_owner->GetNick());
	else if(Owner()->IsMe())
		return StringF(_("You pay $2000 for orders to %s"), ex_owner->GetNick());
	else
		return StringF(_("%s paies $2000 for orders to %s"), Owner()->GetNick(), ex_owner->GetNick());
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
 *                                ECPlane                                                   *
 ********************************************************************************************/
std::string ECPlane::SpecialInfo()
{
	std::string s = !Deployed() ? _("Flying") : _("On ground");

	if(Owner() && Owner()->IsMe())
	{
		if(Containing())
		{
			if(!Deployed())
				s = StringF(_("You pay $%d each turn"), Containing()->Nb() * VolCost());
			else
				s += _(" - Contain:");
		}
		else s += StringF(_(" - Capacity: %d"),100 * Nb());
	}
	else if(Containing())
		s += _(" - Contain:");

	return s;
}

/********************************************************************************************
 *                                ECTrain                                                   *
 ********************************************************************************************/
std::string ECTrain::SpecialInfo()
{
	if(Containing()) return _(" - Contain:");
	else if(Owner() && Owner()->IsMe()) return StringF(_(" - Capacity: %d"),100 * Nb());
	else return "";
}

/********************************************************************************************
 *                                ECBoat                                                    *
 ********************************************************************************************/
std::string ECBoat::SpecialInfo()
{
	if(Containing()) return _(" - Contain:");
	else if(Owner() && Owner()->IsMe()) return StringF(_(" - Capacity: %d"),100 * Nb());
	else return "";
}

/********************************************************************************************
 *                                ECMissiLauncher                                           *
 ********************************************************************************************/

bool ECMissiLauncher::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
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
	switch(EventType())
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
	switch(EventType())
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
	if(EventType() & ARM_MOVE)
	{
		if(!Move()->Empty())
			SetImage(images[(imgs_t)Move()->First()]);
		SetAnim(true);
	}
	return true;
}

bool ECUnit::MoveEffect(const std::vector<ECEntity*>& entities)
{
	ECMap* map = dynamic_cast<ECMap*>(Case()->Map());
	if(Move()->Empty())
	{
		if(move_anim)
			Image()->SetAnim(false);
		return true;
	}

	if(move_anim)
		Image()->SetAnim(true);

	ECMove::E_Move m = Move()->First();
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
			if(Map()->Brouillard() && Case()->Showed() <= 0 || map->ShowMap()->X() + (CASE_WIDTH * int(Case()->X()+1)) <= Image()->X())
				ChangeCase(Case()->MoveRight()), Move()->RemoveFirst(), changed_case = true;
			break;
		case ECMove::Left:
			if(Map()->Brouillard() && Case()->Showed() <= 0 || map->ShowMap()->X() + (CASE_WIDTH * int(Case()->X()-1)) >= Image()->X())
				ChangeCase(Case()->MoveLeft()), Move()->RemoveFirst(), changed_case = true;
			break;
		case ECMove::Down:
			if(Map()->Brouillard() && Case()->Showed() <= 0 || map->ShowMap()->Y() + (CASE_HEIGHT * int(Case()->Y()+1)) <= Image()->Y())
				ChangeCase(Case()->MoveDown()), Move()->RemoveFirst(), changed_case = true;
			break;
		case ECMove::Up:
			if(Map()->Brouillard() && Case()->Showed() <= 0 || map->ShowMap()->Y() + (CASE_HEIGHT * int(Case()->Y()-1)) >= Image()->Y())
				ChangeCase(Case()->MoveUp()), Move()->RemoveFirst(), changed_case = true;
			break;
	}
	if(changed_case && entities.size() == 1 && (!Map()->Brouillard() || Case()->Showed() > 0) &&
	   dynamic_cast<ECMap*>(Case()->Map())->ShowMap() && !IsHiddenOnCase())
	{
		dynamic_cast<ECMap*>(Case()->Map())->ShowMap()->CenterTo(this);
		Map()->ShowWaitMessage.clear();
	}

	if(!Move()->Empty() && m != Move()->First())
		SetImage(images[(imgs_t)Move()->First()]);

	return false;
}

bool ECUnit::MakeEvent(const std::vector<ECEntity*>& entities, ECase*, EC_Client*)
{
	switch(EventType())
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
			if(EventType() & ARM_MOVE || EventType() & ARM_ATTAQ)
				return MoveEffect(entities);
			break;

	}
	return true;
}

bool ECUnit::AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*)
{
	if(EventType() & ARM_ATTAQ)
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
	if(EventType() & ARM_MOVE)
			SetAnim(false);
	return true;
}

void ECUnit::SetDeployed(bool d)
{
	ECBEntity::SetDeployed(d);
	if(Deployed() && images[I_Deployed])
		SetImage(images[I_Deployed]);
}
