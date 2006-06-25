/* server/Units.cpp - Units in server
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
#include "Debug.h"
#include "Channels.h"

/********************************************************************************************
 *                               EContainer                                                 *
 ********************************************************************************************/

void EContainer::Union(ECEntity* entity)
{
	ECEntity::Union(entity);

	EContainer* container = dynamic_cast<EContainer*>(entity);

	if(!container) return;

	if(!Containing())
		Contain(container->Containing());
	else if(container->Containing())
	{
		dynamic_cast<ECEntity*>(Containing())->Union(dynamic_cast<ECEntity*>(container->Containing()));
		dynamic_cast<ECEntity*>(container->Containing())->SetShadowed();
	}
}

bool EContainer::WantContain(ECEntity* entity)
{
	if(Containing() || entity->Locked() || entity->Shadowed() || !CanContain(entity))
		return false;

	ECMove::E_Move move;

	if(entity->Case()->X() == Case()->X())
	{
		if(entity->Case()->Y() == Case()->Y()-1)
			move = ECMove::Down;
		else if(entity->Case()->Y() == Case()->Y()+1)
			move = ECMove::Up;
		else
			return false;
	}
	else if(entity->Case()->Y() == Case()->Y())
	{
		if(entity->Case()->X() == Case()->X()-1)
			move = ECMove::Right;
		else if(entity->Case()->X() == Case()->X()+1)
			move = ECMove::Left;
		else
			return false;
	}
	else
		return false;

	if(!entity->WantMove(move, true))
		return false;

	entity->CreateLast();
	Contain(entity);

	return true;
}

bool EContainer::WantUnContain(uint x, uint y)
{
	return false;
}

void EContainer::ReleaseShoot()
{
	if(Containing())
	{
		ECEntity* contained = dynamic_cast<ECEntity*>(Containing());
		contained->Shooted(shooted);
		contained->ReleaseShoot();

		if(!contained->Nb())
		{
			Channel()->SendArm(NULL, contained, ARM_REMOVE);
			Case()->Map()->RemoveAnEntity(contained, USE_DELETE);
			SetContaining(0);
		}
		shooted = 0;
	}
	else
		ECEntity::ReleaseShoot();
}

void EContainer::ChangeCase(ECBCase* new_case)
{
	ECEntity::ChangeCase(new_case);
	if(Containing())
		Containing()->SetCase(Case());
}

bool EContainer::Attaq(std::vector<ECEntity*> entities)
{
	if(Containing())
	{
		Containing()->SetCase(Case());
		return dynamic_cast<ECEntity*>(Containing())->Attaq(entities);
	}
	else
		return ECEntity::Attaq(entities);
}

/********************************************************************************************
 *                               ECMissiLauncher                                            *
 ********************************************************************************************/

bool ECMissiLauncher::WantDeploy()
{
	/** \note On switch sur ce qu'on VEUT mettre */
	switch(!Deployed())
	{
		case false:
			if(EventType() & ARM_ATTAQ)
				return false;
			break;
		case true:
			break;
	}

	CreateLast();

	SetDeployed(!Deployed());
	return true;
}

bool ECMissiLauncher::WantAttaq(uint mx, uint my)
{
	/* Il faut:
	 * - être déployé
	 * - que ça soit notre première action
	 * - qu'on n'ait pas déjà prévu une attaque
	 */
	if(!Deployed() || Last())
		return false;

	/* On n'attaque pas sur notre case */
	if(Case()->X() == mx && Case()->Y() == my)
		return false;

#if 0 // Ancienne méthode de détection de la portée
	uint dx = 0, dy = 0;
	for(uint x=Case()->X(); x != mx; dx++) x < mx ? ++x : --x;
	for(uint y=Case()->Y(); y != my; dy++) y < my ? ++y : --y;

	/* On ne tire que dans un rayon de quatre cases. */
	if(dx > MISSILAUNCHER_PORTY || dy > MISSILAUNCHER_PORTY)
		return false;
#else
	uint d = 0;
	for(uint x=Case()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=Case()->Y(); y != my; d++) y < my ? ++y : --y;

	/* On ne tire que dans un rayon de quatre cases. */
	if(d > MISSILAUNCHER_PORTY)
		return false;
#endif

#if 0 /* Ce détail est géré dans ARMCommand::Exec(), et permet, en cas d'attaque maintenue, d'attaquer tout de
         meme une case sans qu'il y ait d'unité */

	ECBCase* c = (*Case()->Map())(mx, my);

	/* Si il n'y a personne à attaquer on n'attaque pas */
	if(c->Entities()->empty())
		return false;

	std::vector<ECBEntity*> ents = c->Entities()->List();
	std::vector<ECBEntity*>::iterator enti;
	for(enti = ents.begin(); enti != ents.end() &&
		((*enti)->Locked() || !CanAttaq(*enti) || Like(*enti));
		++enti);

	if(enti == ents.end())
		return false;
#endif
	return true;
}

bool ECMissiLauncher::Attaq(std::vector<ECEntity*> entities)
{
	/* C'est une attaque contre moi (probablement sur la meme case).
	 * Effectivement, cette unité ne tire QUE quand on lui en donne l'ordre
	 */
	if(!(EventType() & ARM_ATTAQ))
		return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !Like(*it) && CanAttaq(*it) && (*it)->Case() != Case())
		{
			uint dx = 0, dy = 0;
			for(uint x=Case()->X(); x != (*it)->Case()->X(); dx++) x < (*it)->Case()->X() ? ++x : --x;
			for(uint y=Case()->Y(); y != (*it)->Case()->Y(); dy++) y < (*it)->Case()->Y() ? ++y : --y;
			uint d = dx + dy;
			float coef = 0;
			switch(d)
			{
				case 0: coef = 0.0f; break;
				case 1: coef = 0.8f; break;
				case 2: coef = 2.0f; break;
				case 3: coef = 1.5f; break;
				case 4: coef = 1.0f; break;
				case 5: coef = 0.9f; break;
				case 6: coef = 0.7f; break;
				case 7: coef = 0.5f; break;
				case 8: coef = 0.3f; break;
			}
			uint killed = 0;
			switch((*it)->Type())
			{
				case E_ARMY:
					killed = uint(Nb() * coef); break;
				case E_BOAT:
				case E_CHAR:
					killed = uint(Nb() * 4 * coef); break;
				case E_MISSILAUNCHER:
					killed = uint(Nb() * 2 * coef); break;
				default:
					if((*it)->IsBuilding())
						killed = uint(Nb() * 7 * coef);
					else
					{
						FDebug(W_WARNING, "Shoot d'un type non supporté");
						continue;
					}
			}
			if(!killed) continue;

			Shoot(*it, killed);
			(*Channel()) << "Le lance missile " + LongName() + " dégomme " + (*it)->Qual() + " " +
							(*it)->LongName() + " de " + TypToStr(killed);
		}

#if 0 // Uniquement si on veut qu'un lance-missile se reploie après avoir tiré
	/* Si on est déployé (ça devrait etre le cas !!), on se remet normal pour faire perdre un tour
	 * au joueur si il veut retirer (il faut se redeployer)
	 */
	if(Deployed())
	{
		SetDeployed(false);
		Channel()->SendArm(NULL, this, ARM_DEPLOY);
	}
	else
		FDebug(W_WARNING, "On n'est pas deployé !?");
#endif

	return false;
}

/********************************************************************************************
 *                               ECUnit                                                     *
 ********************************************************************************************/

bool ECUnit::WantAttaq(uint mx, uint my)
{
	/* On ne peut attaquer que si on est sur la case */
	if(Case()->X() != mx || Case()->Y() != my)
		return false;

	/* Si il n'y a personne à attaquer on n'attaque pas */
	if(Case()->Entities()->size() <= 1) // <=1 car je suis forcément dans la liste vu que je suis sur la case
		return false;

	std::vector<ECBEntity*> ents = Case()->Entities()->List();
	std::vector<ECBEntity*>::iterator enti;
	for(enti = ents.begin(); enti != ents.end() &&
		(dynamic_cast<ECEntity*>(*enti)->Shadowed() || !CanAttaq(*enti) || Like(*enti));
		++enti);

	if(enti == ents.end())
		return false;

	return true;
}

bool ECUnit::WantMove(ECBMove::E_Move move, bool force)
{
	/* J'ai déjà fait tous mes pas
	 * Si on est deployé on ne peut pas bouger */
	if(!restStep || Deployed()) return false;

	ECBCase *c = 0;

	switch(move)
	{
		case ECBMove::Up: c = Case()->MoveUp(); break;
		case ECBMove::Down: c = Case()->MoveDown(); break;
		case ECBMove::Left: c = Case()->MoveLeft(); break;
		case ECBMove::Right: c = Case()->MoveRight(); break;
	}
	if(!c || c == Case() || last && last->Case() == c) return false;

	if(!force && !(c->Flags() & case_flags)) return false;

	restStep--;

	/* Si sur la case actuelle il y a une attaque, on ne bouge pas meme si on dit
	 * que si au client.
	 */
	if(!force)
	{
		std::vector<ECBEntity*> ents = Case()->Entities()->List();
		for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
			if(((!dynamic_cast<ECEntity*>(*enti)->Shadowed() && !(*enti)->Last()) ||
				(*enti)->MyStep() - (*enti)->RestStep() == this->MyStep() - this->RestStep())
			&& ThereIsAttaq(*enti, this))
			{
				Move()->AddMove(move);
				return true;
			}
	}

	/* On ajout le move bien après le CreateLast() */
	CreateLast();
	Move()->AddMove(move);
	ChangeCase(c);

	return true;
}
