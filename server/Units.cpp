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
 *                               ECMcDo                                                     *
 ********************************************************************************************/

void ECMcDo::Invest(ECBEntity* entity)
{
	if(Shadowed()) return;

	ECEntity* enti = dynamic_cast<ECEntity*>(entity);

	/* Pour le client, on supprime à la fois la caserne et le mcdonald */
	Channel()->SendArm(0, enti, ARM_REMOVE|ARM_INVEST);
	Channel()->SendArm(0, this, ARM_REMOVE|ARM_INVEST);

	/* On se met à autant que la caserne */
	SetNb(enti->Nb());

	/* On se rappelle de la caserne, de son ancien owner puis on se définit un nouvel owner */
	caserne = enti;
	ex_owner = Owner();
	SetOwner(enti->Owner());
	if(ex_owner)
		ex_owner->Entities()->Remove(this);
	else
		Map()->Neutres()->Remove(this);
	if(Owner())
		Owner()->Entities()->Add(this);
	else
		Map()->Neutres()->Add(this);

	/* On change d'identité maintenant qu'on "appartient" à l'owner de la caserne, et on réapparait */
	SetID(Channel()->FindEntityName(Owner()));
	Channel()->SendArm(NULL, this, ARM_CREATE|ARM_HIDE, Case()->X(), Case()->Y());

	/* On se déploie pour avoir le status batiment */
	SetDeployed();
	Channel()->SendArm(0, this, ARM_DEPLOY);

	Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_INVESTED, TypToStr(caserne->Type())));
	Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_EXOWNER, ex_owner ? ex_owner->GetNick() : "McGerbale neutre"));

	/* On supprime de la circulation la caserne */
	Map()->RemoveAnEntity(caserne);
}

bool ECMcDo::CanCreate(const ECBEntity* entity)
{
	if(!Deployed() || !caserne) return false;

	return caserne->CanCreate(entity);
}

int ECMcDo::TurnMoney(ECBPlayer* pl)
{
	if(!Deployed()) return 0;

	if(ex_owner == Owner())
		return 0; /* En effet, on a du investir le McDo avec un ingénieur ou conquérir la ville un truc du genre */

	if(ex_owner == pl)
		return 1000;
	else if(Owner() == pl)
		return -1000;
	else
		return 0;
}

void ECMcDo::ChangeOwner(ECBPlayer* pl)
{
	ECEntity::ChangeOwner(pl);

	if(!Deployed() || !caserne) return;

	Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_INVESTED, TypToStr(caserne->Type())));
	Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_EXOWNER, ex_owner ? ex_owner->GetNick() : "McGerbale neutre"));
}

/********************************************************************************************
 *                               ECEnginer                                                  *
 ********************************************************************************************/

void ECEnginer::Invest(ECBEntity* entity)
{
	/* On a été tué mais bon faudrait quand même que le test ne se fasse pas ici, le problème est que la fonction
	 * qui nous appelle ici est dans la lib (ECBCase::CheckInvests()). Elle n'est appelée que par le serveur,
	 * et est dans la lib parce que la classe ECBCase n'est pas dérivée dans le serveur. Il faudrait trouver
	 * une alternative
	 */
	if(Shadowed()) return;
	if(entity->Owner() == Owner())
	{
		if(entity->Nb() >= entity->InitNb()) return;
		entity->SetNb(entity->InitNb());
		Channel()->SendArm(Owner()->Client(), dynamic_cast<ECEntity*>(entity), ARM_NUMBER);
	}
	else
	{
		ECEntity::Invest(entity);
	
		if(entity->IsCountryMaker())
			return;
	}

	Channel()->SendArm(0, this, ARM_REMOVE|ARM_INVEST);
	SetShadowed();
	/* On attend de se faire purger */
}

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

bool EContainer::WantContain(ECEntity* entity, ECMove::Vector& moves)
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

	if(!entity->WantMove(move, MOVE_FORCE))
		return false;

	entity->CreateLast();
	CreateLast();
	Contain(entity);
	moves.push_back(move);

	return true;
}

bool EContainer::WantUnContain(uint x, uint y, ECMove::Vector& moves)
{
	if(!Containing() || !restStep || !Containing()->RestStep())
		return false;

	ECMove::E_Move move;

	if(x == Case()->X())
	{
		if(y == Case()->Y()-1)
			move = ECMove::Up;
		else if(y == Case()->Y()+1)
			move = ECMove::Down;
		else
			return false;
	}
	else if(y == Case()->Y())
	{
		if(x == Case()->X()-1)
			move = ECMove::Left;
		else if(x == Case()->X()+1)
			move = ECMove::Right;
		else
			return false;
	}
	else
		return false;

	Containing()->SetCase(Case());
	if(!Containing()->WantMove(move, MOVE_SIMULE))
		return false;

	CreateLast();

	ECBEntity* entity = Containing();
	UnContain();

	if(!entity->WantMove(move, MOVE_FORCE))
		throw ECExcept(VIName(move) VIName(x) VIName(y) VIName(Case()->X()) VIName(Case()->Y()), "Gros problème là");

	moves.push_back(move);

	return true;
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

bool EContainer::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	if(Containing())
	{
		Containing()->SetCase(Case());
		return dynamic_cast<ECEntity*>(Containing())->Attaq(entities, event);
	}
	else
		return ECEntity::Attaq(entities, event);
}

/********************************************************************************************
 *                               ECMissiLauncher                                            *
 ********************************************************************************************/

bool ECMissiLauncher::WantDeploy()
{
	if(EventType() & ARM_DEPLOY)
		return false;

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

bool ECMissiLauncher::WantAttaq(uint mx, uint my, bool force)
{
	/* Il faut:
	 * - être déployé
	 * - que ça soit notre première action
	 * - qu'on n'ait pas déjà prévu une attaque
	 */
	if(!Deployed() || !force && Last())
		return false;

	/* On n'attaque pas sur notre case */
	if(Case()->X() == mx && Case()->Y() == my)
		return false;

	uint d = 0;
	for(uint x=Case()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=Case()->Y(); y != my; d++) y < my ? ++y : --y;

	/* On ne tire que dans un rayon de quatre cases. */
	if(d > Porty())
		return false;

	return true;
}

bool ECMissiLauncher::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case).
	 * Effectivement, cette unité ne tire QUE quand on lui en donne l'ordre
	 */
	if(!(EventType() & ARM_ATTAQ) || event->Case() == Case())
		return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !Like(*it) && CanAttaq(*it) && (*it)->Case() != Case() && (*it)->Nb())
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
			if((*it)->IsBuilding())                          killed = uint(Nb() * 3 * coef);
			else if((*it)->IsInfantry())                     killed = uint(Nb() * coef);
			else if((*it)->IsVehicule() || (*it)->IsNaval()) killed = uint(Nb() * 2 * coef);
			else if((*it)->Type() == E_MISSILAUNCHER)        killed = uint(Nb() * coef);
			else
			{
				FDebug(W_WARNING, "Shoot d'un type non supporté");
				continue;
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

bool ECUnit::WantAttaq(uint mx, uint my, bool force)
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

bool ECUnit::WantMove(ECBMove::E_Move move, int flags)
{
	/* J'ai déjà fait tous mes pas
	 * Si on est deployé on ne peut pas bouger */
	if(!restStep && !(flags & MOVE_SIMULE) || Deployed() || !Case()) return false;

	ECBCase *c = 0;

	switch(move)
	{
		case ECBMove::Up: c = Case()->MoveUp(); break;
		case ECBMove::Down: c = Case()->MoveDown(); break;
		case ECBMove::Left: c = Case()->MoveLeft(); break;
		case ECBMove::Right: c = Case()->MoveRight(); break;
	}
	if(!c || c == Case() || last && last->Case() == c) return false;

	if(!(flags & MOVE_FORCE) && !CanWalkOn(c)) return false;

	if(flags & MOVE_SIMULE)
		return true;

	restStep--;

	/* Si sur la case actuelle il y a une attaque, on ne bouge pas meme si on dit
	 * que si au client.
	 */
	if(!(flags & MOVE_FORCE))
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
	if(Move()->Empty())
		Move()->SetFirstCase(Case());
	Move()->AddMove(move);
	ChangeCase(c);

	return true;
}
