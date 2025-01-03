/* server/Map.cpp - Map classes
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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

#include "Defines.h"
#include "Map.h"
#include "Debug.h"
#include "Channels.h"
#include "Main.h"
#include "Batiments.h"
#include <fstream>
#include <algorithm>

MapVector MapList;
MapVector MissionList;

/********************************************************************************************
 *                                 ECMap                                                    *
 ********************************************************************************************/

bool ECMap::LoadMaps()
{
	std::vector<std::string> maps = GetFileList(PKGDATADIR, "map");

	uint nbmaps = 0, nbmissions = 0;

	for(std::vector<std::string>::reverse_iterator ligne = maps.rbegin(); ligne != maps.rend(); ++ligne)
	{
		ECMap *map = 0;
		try
		{
			map = new ECMap(std::string(PKGDATADIR + *ligne));
			map->Init();
		}
		catch(TECExcept &e)
		{
			delete map;
			if(!(app.HasFlag(ECServer::F_SILENT)))
			{
				Debug(W_ERR|W_ECHO, "Unable to load this map file : %s", ligne->c_str());
				vDebug(W_ERR|W_ECHO, e.Message(), e.Vars());
			}
			continue;
		}
		map->i = map->IsMission() ? nbmissions++ : nbmaps++;
		(map->IsMission() ? MissionList : MapList).push_back(map);
	}
	if(!(app.HasFlag(ECServer::F_SILENT)))
		Debug(W_ECHO|W_NOLOG, "%d maps and %d missions loaded !", nbmaps, nbmissions);
	return true;
}

ECMap::~ECMap()
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end(); ++it)
		delete *it;
}

EventVector::iterator ECMap::RemoveEvent(ECEvent* p, bool use_delete)
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end();)
	{
		if (*it == p)
		{
			if(use_delete)
				delete p;
			it = map_events.erase(it);
			return it;
		}
		else
			++it;
	}
	return EventVector::iterator(0);
}

ECEvent* ECMap::FindEvent(ECBCase* c, uint f, ECEntity* e)
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end(); ++it)
		if((!c || (*it)->Case() == c) && (!f || (*it)->Flags() == f) && (!e || (*it)->Entities()->Find(e)))
			return *it;
	return 0;
}

void ECMap::RemoveAnEntity(ECBEntity* e, bool use_delete)
{
	if(!Channel()) throw ECExcept(VPName(Channel()), "Pas de channel lié !?");

	BPlayerVector players = Channel()->Players();
	for(BPlayerVector::iterator pl = players.begin(); pl != players.end(); ++pl)
		if(dynamic_cast<ECPlayer*>(*pl)->Client())
			dynamic_cast<ECPlayer*>(*pl)->Client()->RemoveEntity(e);

	if(Channel() && e->Case() && e->IsCountryMaker() && e->Case()->Country()->Owner() != NULL)
	{
		std::vector<ECBCase*> cases = e->Case()->Country()->Cases();
		size_t nb_makers = 0;
		for(BCaseVector::iterator c = cases.begin(); c != cases.end(); ++c)
		{
			std::vector<ECBEntity*> ents = (*c)->Entities()->List();
			for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
				if(*it != e && !(*it)->IsZombie() && (*it)->IsCountryMaker())
					nb_makers++;
		}
		if(nb_makers == 0)
			e->Case()->Country()->ChangeOwner(NULL);
	}

	ECBMap::RemoveAnEntity(e, use_delete);
}

template<typename T>
static ECBCase* CreateCase(ECBMap *map, uint x, uint y, uint flags, char type_id)
{
	return new T(map, x, y, flags, type_id);
}

static struct
{
	char c;
	ECBCase* (*func) (ECBMap *map, uint x, uint y, uint flgs, char type_id);
	uint flags;
} case_type[] = {
	{ 'm', CreateCase<ECMer>,      C_MER              },
	{ 't', CreateCase<ECTerre>,    C_TERRE            },
	{ 'p', CreateCase<ECPont>,     C_PONT             },
	{ 'M', CreateCase<ECMontain>,  C_MONTAIN          }
};

ECBCase* ECMap::CreateCase(uint _x, uint _y, char type_id)
{
	for(uint j=0; j < (sizeof case_type / sizeof *case_type); j++)
		if(case_type[j].c == type_id)
			return case_type[j].func (this, _x, _y, case_type[j].flags, case_type[j].c);

	return 0;
}

/********************************************************************************************
 *                                 ECase                                                    *
 ********************************************************************************************/
void ECase::CheckInvests(ECBEntity* e)
{
	if(!e->Owner()) return;
	std::vector<ECBEntity*> ents = entities.List();
	for(std::vector<ECBEntity*>::const_iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		if(dynamic_cast<ECEntity*>(*enti)->IsZombie())
			continue;
		if(e->CanInvest(*enti))
		{
			e->Invest(*enti);
			break;
		}
		else if((*enti)->CanInvest(e))
		{ /* On sait jamais, si par exemple une unité peut en investir une autre, mais que c'est cette autre qui se deplace
		   * vers la mienne, ou si les deux se déplacent dans le même tour sur la meme case, ça serait con que juste
		   * parce que l'unité investisseuse s'est déplacée *avant* l'unité à investir, il n'y ait pas d'investiture
		   */
			(*enti)->Invest(e);
			break;
		}
	}
}

/********************************************************************************************
 *                                 ECEvent                                                  *
 ********************************************************************************************/

bool ECEvent::RemoveLinked(ECEvent* p, bool use_delete)
{
	for(EventVector::iterator it = linked.begin(); it != linked.end();)
	{
		if (*it == p)
		{
			if(use_delete)
				delete p;
			it = linked.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

/********************************************************************************************
 *                                 ECountry                                                 *
 ********************************************************************************************/

bool ECountry::ChangeOwner(ECBMapPlayer* mp)
{
	ECBMapPlayer* last_owner = owner;
	if(ECBCountry::ChangeOwner(mp))
	{
		for(BCaseVector::iterator c = cases.begin(); c != cases.end(); ++c)
		{
			std::vector<ECBEntity*> ents = (*c)->Entities()->List();
			for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
				if(!(*it)->IsZombie() && (*it)->IsCity())
					(*it)->ChangeOwner(owner ? owner->Player() : 0);
		}
		if(owner && owner->Player())
		{
			EChannel* chan = dynamic_cast<EChannel*>(owner->Player()->Channel());
			chan->sendto_players(0, owner->Player(), MSG_SET, ECArgs("+@", ident));
		}
		else if(last_owner && last_owner->Player())
		{
			EChannel* chan = dynamic_cast<EChannel*>(last_owner->Player()->Channel());
			chan->sendto_players(0, last_owner->Player(), MSG_SET, ECArgs("-@", ident));
		}
		else
			Debug(W_WARNING, "Country %s changed from nobody to nobody??", ID());
		return true;
	}
	return false;
}


/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/

void ECEntity::Invest(ECBEntity* e)
{
	if(!Owner())
		Debug(W_WARNING, "ECEntity::Invest(): A neutral unity (%d) invests an other entity (%s: %d) !?!?!?", Type(), e->LongName().c_str(), e->Type());
	e->ChangeOwner(Owner());
}

void ECEntity::ChangeOwner(ECBPlayer* pl)
{
	if(Owner() == pl) return;
	ECPlayer* last_owner = Owner();
	EChannel* c = Channel();
	c->SendArm(NULL, this, ARM_REMOVE|ARM_INVEST);
	SetOwner(pl);

	if(last_owner)
		last_owner->Entities()->Remove(this);
	else
		c->Map()->Neutres()->Remove(this);
	if(Owner())
		Owner()->Entities()->Add(this);
	else
		c->Map()->Neutres()->Add(this);

	SetID(c->FindEntityName(Owner()));
	c->SendArm(NULL, this, ARM_CREATE|ARM_HIDE|ARM_INVEST, Case()->X(), Case()->Y());
	if(Deployed())
		c->SendArm(NULL, this, ARM_DEPLOY);

	/* On change de propriétaire les evenements */
	EventVector events = Events()->List();
	for(EventVector::iterator evti = events.begin(); evti != events.end(); ++evti)
	{
		if(last_owner)
			last_owner->Events()->Remove(*evti);
		if(Owner())
			Owner()->Events()->Add(*evti);
	}

	if(IsCountryMaker())
		Case()->Country()->ChangeOwner(Owner() ? Owner()->MapPlayer() : 0);
}

bool ECEntity::AreFriends(std::vector<ECEntity*> list)
{
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
		for(std::vector<ECEntity*>::iterator ti = list.begin(); ti != list.end(); ++ti)
			if(!(*ti)->Like(*it) && !(*it)->Like(*ti))
				return false;
	return true;
}

ECEntity::~ECEntity()
{

}

void ECEntity::Shoot(ECEntity* e, uint k)
{
	Debug(W_DEBUG, "%s shoot %s de %d", LongName().c_str(), e->LongName().c_str(), k);
	e->Shooted(k);

	if (!Owner())
		return;

	if (k > e->Nb())
		k = e->Nb();

	Owner()->Stats()->killed += k;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
	{
		ECGulag* gulag = dynamic_cast<ECGulag*>(*it);
		if (!gulag)
			continue;
		if (gulag->NbPrisoners() + k > gulag->MaxPrisoners())
		{
			k -= gulag->MaxPrisoners() - gulag->NbPrisoners();
			gulag->AddPrisoners(gulag->MaxPrisoners() - gulag->NbPrisoners());
		}
		else
		{
			gulag->AddPrisoners(k);
			return;
		}
	}
}

void ECEntity::ReleaseShoot()
{
	SetNb(Nb() - (shooted > Nb() ? Nb() : shooted));
	if(Owner())
		Owner()->Stats()->losses += shooted;
	shooted = 0;
}

ECPlayer* ECEntity::Owner() const
{
	return dynamic_cast<ECPlayer*>(ECBEntity::Owner());
}

ECMap* ECEntity::Map() const
{
	return dynamic_cast<ECMap*>(ECBEntity::Map());
}

EChannel* ECEntity::Channel() const
{
	assert(Map());
	assert(Map()->Channel());
	return dynamic_cast<EChannel*>(Map()->Channel());
}

void ECEntity::Union(ECEntity* entity)
{
	assert(entity->Type() == Type());

	/* On lock car il fait maintenant partie intégrale de (*enti) */
	entity->SetZombie();

	/* On met dans le nouvel etat de l'entité le nouveau nombre de soldats */
	SetNb(Nb() + entity->Nb());
	/* Enfin on défini le nombre de pas restants */
	SetRestStep(RestStep() > (entity->MyStep() - entity->RestStep()) ? RestStep() - (entity->MyStep() - entity->RestStep())
	                                                                 : 0);

	/* On récupère les evenements */
	std::vector<ECEvent*> events = entity->Events()->List();
	FORit(ECEvent*, events, evti)
	{
		Events()->Add(*evti);
		if((*evti)->Entity() == entity)
			(*evti)->SetEntity(this);

		if((*evti)->Entities()->Remove(entity))
			(*evti)->Entities()->Add(this);
	}
}

void ECEntity::Played()
{
}

bool ECEntity::Attaq(std::vector<ECEntity*> entities, ECEvent*)
{
	uint enemies = 0;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(this != *it && (*it)->Case() == Case() && !Like(*it) && (*it)->Nb() &&
		   (CanAttaq(*it) || (Parent() && Parent()->CanAttaq(*it))))
			enemies++;

	if(!enemies) return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && (*it)->Case() == Case() && !Like(*it) && (CanAttaq(*it) || (Parent() && Parent()->CanAttaq(*it))))
		{
			// Thomas' adaptation
			uint attaq_nb = Nb() > 1000 ? 1000 : Nb();

			uint killed = rand() % (attaq_nb/(2+enemies)+1); // +1 pour pas risquer d'avoir un modulo 0 (possible si nb < 2+enemies).

			if(killed < attaq_nb/(4+enemies)) killed = attaq_nb/(4+enemies);
			if(!killed)
				return false;
			Shoot(*it, killed);
		}

	return true;
}

std::nrvector<TClient*> ECEntity::EntitiesToClients(std::vector<ECEntity*> entities)
{
	std::nrvector<TClient*> players;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if((*it)->Owner() && (*it)->Owner()->Client())
		{
			std::vector<ECBPlayer*> allies = (*it)->Owner()->Allies();
			FORit(ECBPlayer*, allies, al)
				if(dynamic_cast<ECPlayer*>(*al)->Client())
					players.push_back(dynamic_cast<ECPlayer*>(*al)->Client());
			players.push_back((*it)->Owner()->Client());
		}
	return players;
}

void ECEntity::CancelEvents()
{
	ECMap* map = dynamic_cast<ECMap*>(Map());
	EventVector events = Events()->List();
	std::vector<ECEntity*> entities;
	for(EventVector::iterator evti = events.begin(); evti != events.end(); ++evti)
	{
		std::vector<ECEntity*> ents = (*evti)->Entities()->List();
		if((*evti)->Entity() != this && (*evti)->Entity()->Owner() == Owner())
		{
			(*evti)->Entity()->Events()->Remove(*evti);
			if((*evti)->Entity()->Owner())
				(*evti)->Entity()->Owner()->Events()->Remove(*evti);
			entities.push_back((*evti)->Entity());
		}
		FORit(ECEntity*, ents, enti)
			if(*enti != this && (*enti)->Owner() == Owner())
			{
				(*enti)->Events()->Remove(*evti);
				if((*enti)->Owner())
					(*enti)->Owner()->Events()->Remove(*evti);
				entities.push_back(*enti);
			}

		if(Owner())
			Owner()->Events()->Remove(*evti);
		map->RemoveEvent(*evti, USE_DELETE);
	}

	Move()->Clear(Case());
	Events()->Clear();

	ECBEntity::CancelEvents();

	// On annule les evenements des unités liées à ces evenements
	FOR(ECEntity*, entities, enti)
	{
		enti->CancelEvents();
		if (enti->Owner() && enti->Owner()->Client())
			Channel()->SendArm(enti->Owner()->Client(), enti, ARM_RETURN,
			                   enti->Case()->X(), enti->Case()->Y());
	}
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
		Channel()->SendArm(0, dynamic_cast<ECEntity*>(Containing()), ARM_CONTENER);
		container->SetUnit(0);
	}
	else if(container->Containing())
	{
		dynamic_cast<ECEntity*>(Containing())->Union(dynamic_cast<ECEntity*>(container->Containing()));
		Channel()->SendArm(0, dynamic_cast<ECEntity*>(container->Containing()), ARM_REMOVE|ARM_INVEST);
		if(Owner() && Owner()->Client())
			Channel()->SendArm(Owner()->Client(), dynamic_cast<ECEntity*>(Containing()), ARM_NUMBER);
		container->Containing()->SetParent(0);
		container->SetUnit(0);
	}
}

bool EContainer::Contain(ECBEntity* entity)
{
	if(Containing())
	{
		if(entity->Type() != Containing()->Type())
		{
			Debug(W_WARNING, "EContainer::Contain(): %d veut entrer dans un %d qui contient un %d !", entity->Type(), Type(), Containing()->Type());
			return false;
		}
		Containing()->SetNb(Containing()->Nb() + entity->Nb());
		dynamic_cast<ECEntity*>(entity)->SetZombie();
		/*
		 * Channel()->SendArm(0, dynamic_cast<ECEntity*>(entity), ARM_REMOVE|ARM_INVEST);
		 *
		 * On l'envoie par la suite
		 */
		if(Owner() && Owner()->Client())
			Channel()->SendArm(Owner()->Client(), dynamic_cast<ECEntity*>(Containing()), ARM_NUMBER);

		return true;
	}

	return ECBContainer::Contain(entity);
}

bool EContainer::WantContain(ECEntity* entity, ECMove::Vector& moves)
{
	if((Containing() && Containing()->Type() != entity->Type()) || entity->Locked() || entity->IsZombie() || !CanContain(entity))
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

	std::vector<ECEvent*> events = Events()->List();
	uint contened = Containing() ? Containing()->Nb() : 0;
	FORit(ECEvent*, events, event)
		if((*event)->Flags() & ARM_CONTENER)
		{
			if((*event)->Entity()->Type() != entity->Type())
				return false;
			contened += (*event)->Entity()->Nb();
		}

	if(contened + entity->Nb() > UnitaryCapacity() * Nb())
		return false;

	if(!entity->WantMove(move, MOVE_FORCE))
		return false;

	moves.push_back(move);

	return true;
}

bool EContainer::WantUnContain(uint x, uint y, ECMove::Vector& moves)
{
	if(!Containing() || !Containing()->RestStep())
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
	if(!Containing()->CanWalkOn((*Map())(x,y)) || !Containing()->WantMove(move, MOVE_FORCE))
		return false;

	moves.push_back(move);

	return true;
}

void EContainer::ReleaseShoot()
{
	if(Containing())
	{
		ECEntity* contained = dynamic_cast<ECEntity*>(Containing());
		int contained_nb = contained->Nb();
		contained->Shooted(shooted);
		contained->ReleaseShoot();

		if(!contained->Nb())
		{
			Channel()->SendArm(NULL, contained, ARM_REMOVE);
			contained->SetZombie();
			UnContain();

			/* Si jamais ce qu'on contenait est détruit, on reporte sur le bateau */
			shooted -= contained_nb;
			ECEntity::ReleaseShoot();
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

