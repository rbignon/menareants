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
#include "InGame.h"

/********************************************************************************************
 *                               ECPlane                                                    *
 ********************************************************************************************/

bool ECPlane::WantDeploy()
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

	return true;
}

bool ECPlane::WantUnContain(uint x, uint y, ECMove::Vector& moves)
{
	if(!Deployed() && !(EventType() & ARM_DEPLOY)) return false;

	return EContainer::WantUnContain(x, y, moves);
}

int ECPlane::TurnMoney(ECBPlayer* pl)
{
	if(Deployed() || !Containing() || Owner() != pl) return 0;

	return - (Containing()->Nb() * VolCost());
}

/********************************************************************************************
 *                               ECJouano                                                   *
 ********************************************************************************************/
void ECJouano::Invest(ECBEntity* entity)
{
	ECMcDo* mcdo = dynamic_cast<ECMcDo*>(entity);

	assert(mcdo);

	Channel()->send_info(0, EChannel::I_JOUANO, ECArgs(LongName(), mcdo->ExOwner()->Nick(), mcdo->LongName(), TypToStr(JOUANO_DESTROYTURN)));

	Channel()->SendArm(0, this, ARM_REMOVE|ARM_INVEST);

	mcdo->RestDestroy() = JOUANO_DESTROYTURN + 1;

	/* >  Channel()->SendArm(0, entity, ARM_DATA, 0, 0, ECData(DATA_JOUANO, entity->RestDestroy()));
	 * Inutile: à la fin du tour, ECMcDo::Played() sera appellé et l'enverra de lui même.
	 * C'est d'ailleurs pour cela que l'on fait JOUANO_DESTROYTURN + 1, car la fonction en question
	 * va decrementer la variable.
	 */

	SetZombie();
}

bool ECJouano::WantDeploy()
{
	if((EventType() & ARM_ATTAQ) || Deployed())
		return false;

	Debug(W_DEBUG, "Création d'un evenement ATTAQ");
	ECEvent* event = new ECEvent(ARM_ATTAQ, this, DestCase());

	Map()->AddEvent(event);
	if(Owner())
	{
		if(Owner()->Client())
			Channel()->SendArm(Owner()->Client(), this, ARM_DEPLOY);
		Owner()->Events()->Add(event);
	}
	AddEvent(ARM_ATTAQ);
	Events()->Add(event);

	return false;
}

bool ECJouano::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case). */
	if(!(EventType() & ARM_ATTAQ))
		return ECEntity::Attaq(entities, event);

	ECBCase* c = event->Case();
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && (*it)->Case() != c && (*it)->IsInfantry())
			Shoot(*it, 500-rand()%100-(*it)->Case()->Delta(Case()));

	Channel()->send_info(0, EChannel::I_JOUANO_FART, LongName());

	SetDeployed(false);

	return false;
}

std::vector<ECBEntity*> ECJouano::GetAttaquedEntities(ECBCase* c) const
{
	std::vector<ECBEntity*> entities;
	ECBCase* cc = c->MoveLeft(Porty())->MoveUp(Porty());
	for(uint i=0; i <= 2*Porty(); ++i)
	{
		uint j=0;
		for(; j <= 2*Porty(); ++j)
		{
			std::vector<ECBEntity*> ents = cc->Entities()->List();
			entities.insert(entities.end(), ents.begin(), ents.end());

			if(cc->X() == cc->Map()->Width()-1)
				break;
			cc = cc->MoveRight();
		}
		if(cc->Y() == cc->Map()->Height()-1)
			break;
		cc = cc->MoveDown();
		cc = cc->MoveLeft(j);
	}
	return entities;
}

/********************************************************************************************
 *                               ECMcDo                                                     *
 ********************************************************************************************/

void ECMcDo::Resynch(ECPlayer* pl)
{
	if(caserne)
	{
		Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_INVESTED, TypToStr(caserne->Type())));
		Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_EXOWNER, ex_owner ? ex_owner->GetNick() : "McGerbale neutre"));
	}
	if(restDestroy > 0)
		Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_JOUANO, TypToStr(restDestroy)));
}

void ECMcDo::Invest(ECBEntity* entity)
{
	if(IsZombie()) return;

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

	/* On enlève caserne de partout, mais on ne libère PAS la mémoire ! */
	Map()->RemoveAnEntity(caserne);

	caserne->SetZombie(false); // on sait jamais
}

void ECMcDo::Played()
{
	if(Deployed() && restDestroy > 0)
	{
		restDestroy--;
		if(restDestroy > 0)
			Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_JOUANO, TypToStr(restDestroy)));
		else
		{
			/* On supprime le malheureux McDo, et on restaure la caserne */

			// Suppression propagée de notre mcdo
			Channel()->SendArm(0, this, ARM_REMOVE);

			MyFree(caserne); // on supprime la caserne qu'on avait gardé et qui n'est plus dans aucune liste

			/* On créé notre caserne avec les propriétés du McDo, et je rappelle que Owner() est bien l'owner
			 * de l'ex caserne
			 */
			ECEntity* new_caserne = CreateAnEntity(E_CASERNE, Channel()->FindEntityName(Owner()), Owner(), Case());
			Channel()->SendArm(0, new_caserne, ARM_CREATE, Case()->X(), Case()->Y());
			new_caserne->SetNb(Nb());
			Map()->AddAnEntity(caserne);
			new_caserne->Create(false);

			SetZombie(); // Et on supprime notre brave McDo qui a rendu de mauvais services
		}
	}
	ECEntity::Played();
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
		return 2000;
	else if(Owner() == pl)
		return -2000;
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
	 * une alternative.
	 */
	if(IsZombie()) return;
	if(entity->Owner() == Owner() && Owner()->Client())
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
	SetZombie();
	/* On attend de se faire purger */
}

/********************************************************************************************
 *                               EContainer                                                 *
 ********************************************************************************************/

void EContainer::Union(ECEntity* entity)
{
	ECEntity::Union(entity);

	EContainer* container = dynamic_cast<EContainer*>(entity);

	if(!container || !container->Containing()) return;

	if(!Containing())
	{
		Contain(container->Containing());
		Channel()->SendArm(0, dynamic_cast<ECEntity*>(container->Containing()), ARM_CONTENER);
	}
	else if(container->Containing())
	{
		dynamic_cast<ECEntity*>(Containing())->Union(dynamic_cast<ECEntity*>(container->Containing()));
		Channel()->SendArm(0, dynamic_cast<ECEntity*>(container->Containing()), ARM_REMOVE|ARM_INVEST);
		if(Owner() && Owner()->Client())
			Channel()->SendArm(Owner()->Client(), dynamic_cast<ECEntity*>(Containing()), ARM_NUMBER);
	}
}

bool EContainer::Contain(ECBEntity* entity)
{
	if(Containing())
	{
		Containing()->SetNb(Containing()->Nb() + entity->Nb());
		dynamic_cast<ECEntity*>(entity)->SetZombie();
		Channel()->SendArm(0, dynamic_cast<ECEntity*>(entity), ARM_REMOVE|ARM_INVEST);
		if(Owner() && Owner()->Client())
			Channel()->SendArm(Owner()->Client(), dynamic_cast<ECEntity*>(Containing()), ARM_NUMBER);

		return true;
	}

	return ECBContainer::Contain(entity);
}

bool EContainer::WantContain(ECEntity* entity, ECMove::Vector& moves)
{
	if(Containing() || entity->Locked() || entity->IsZombie() || !CanContain(entity))
		return false;

	ECMove::E_Move move;
	ECBCase* c = DestCase();

	if(entity->DestCase()->X() == c->X())
	{
		if(entity->DestCase()->Y() == c->Y()-1)
			move = ECMove::Down;
		else if(entity->DestCase()->Y() == c->Y()+1)
			move = ECMove::Up;
		else
			return false;
	}
	else if(entity->DestCase()->Y() == c->Y())
	{
		if(entity->DestCase()->X() == c->X()-1)
			move = ECMove::Right;
		else if(entity->DestCase()->X() == c->X()+1)
			move = ECMove::Left;
		else
			return false;
	}
	else
		return false;

	if(!entity->WantMove(move, MOVE_FORCE))
		return false;

	moves.push_back(move);

	return true;
}

bool EContainer::WantUnContain(uint x, uint y, ECMove::Vector& moves)
{
	if(!Containing() || !restStep || !Containing()->RestStep())
		return false;

	ECMove::E_Move move;
	ECBCase* c = DestCase();

	if(x == c->X())
	{
		if(y == c->Y()-1)
			move = ECMove::Up;
		else if(y == c->Y()+1)
			move = ECMove::Down;
		else
			return false;
	}
	else if(y == c->Y())
	{
		if(x == c->X()-1)
			move = ECMove::Left;
		else if(x == c->X()+1)
			move = ECMove::Right;
		else
			return false;
	}
	else
		return false;

	Containing()->SetCase(c);
	if(!Containing()->WantMove(move, MOVE_FORCE))
		return false;

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
			contained->SetZombie();
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

	return true;
}

bool ECMissiLauncher::WantAttaq(uint mx, uint my, bool force)
{
	/* Il faut être déployé ou vouloir se deployer
	 */
	if(!Deployed() ^ !!(EventType() & ARM_DEPLOY))
		return false;

	/* On n'attaque pas sur notre case */
	if(DestCase()->X() == mx && DestCase()->Y() == my)
		return false;

	uint d = 0;
	for(uint x=DestCase()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=DestCase()->Y(); y != my; d++) y < my ? ++y : --y;

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
		if(*it != this && !Like(*it) && CanAttaq(*it) && (*it)->Case() != Case() && (*it)->Nb() && (*it)->Level() <= L_GROUND)
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
			else if((*it)->IsVehicule() || (*it)->IsNaval()
			        || (*it)->IsPlane())                     killed = uint(Nb() * 2 * coef);
			else if((*it)->Type() == E_MISSILAUNCHER)        killed = uint(Nb() * coef);
			else
			{
				FDebug(W_WARNING, "Shoot d'un type non supporté");
				continue;
			}
			if(!killed) continue;

			Shoot(*it, killed);
			if(Owner())
				Channel()->send_info(Owner(), EChannel::I_SHOOT, ECArgs(LongName(), (*it)->LongName(), TypToStr(killed)));
			if((*it)->Owner())
				Channel()->send_info((*it)->Owner(), EChannel::I_SHOOT, ECArgs(LongName(), (*it)->LongName(), TypToStr(killed)));
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
	ECBCase *here = DestCase();

	/* On ne peut attaquer que si on est sur la case */
	if(here->X() != mx || here->Y() != my)
		return false;

	/* Si il n'y a personne à attaquer on n'attaque pas */
	if(here->Entities()->Size() <= 1) // <=1 car je suis forcément dans la liste vu que je suis sur la case
		return false;

	std::vector<ECBEntity*> ents = here->Entities()->List();
	std::vector<ECBEntity*>::iterator enti;
	for(enti = ents.begin(); enti != ents.end() &&
		(dynamic_cast<ECEntity*>(*enti)->IsZombie() || !CanAttaq(*enti) || Like(*enti));
		++enti);

	if(enti == ents.end())
		return false;

	return true;
}

bool ECUnit::WantMove(ECBMove::E_Move move, int flags)
{
	/* J'ai déjà fait tous mes pas
	 * Si on est deployé on ne peut pas bouger */
	if(!restStep && !(flags & MOVE_SIMULE) || Deployed() ^ !!(EventType() & ARM_DEPLOY) || !Case() || Locked() && !(flags & MOVE_FORCE))
		return false;

	ECBCase *c = 0, *here = DestCase();
	switch(move)
	{
		case ECBMove::Up:
			if(!Move()->Empty() && Move()->Moves().back() == ECBMove::Down)
				return false;
			c = here->MoveUp();
			break;
		case ECBMove::Down:
			if(!Move()->Empty() && Move()->Moves().back() == ECBMove::Up)
				return false;
			c = here->MoveDown();
			break;
		case ECBMove::Left:
			if(!Move()->Empty() && Move()->Moves().back() == ECBMove::Right)
				return false;
			c = here->MoveLeft();
			break;
		case ECBMove::Right:
			if(!Move()->Empty() && Move()->Moves().back() == ECBMove::Left)
				return false;
			c = here->MoveRight();
			break;
	}
	if(!c || c == here) return false;

	if(!(flags & MOVE_FORCE) && !CanWalkOn(c)) return false;

	if(flags & MOVE_SIMULE)
		return true;

	restStep--;

	if(Move()->Empty())
		Move()->SetFirstCase(Case());
	Move()->AddMove(move);

	return true;
}
