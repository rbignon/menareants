/* server/Map.cpp - Map classes
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

#include "Defines.h"
#include "Map.h"
#include "Debug.h"
#include "Channels.h"
#include "Main.h"
#include "Units.h"
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
	if(e->IsCountryMaker())
		Case()->Country()->ChangeOwner(Owner() ? Owner()->MapPlayer() : 0);
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
}

bool ECEntity::AreFriends(std::vector<ECEntity*> list)
{
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
		for(std::vector<ECEntity*>::iterator ti = list.begin(); ti != list.end(); ++ti)
			if(!(*ti)->Like(*it) && !(*it)->Like(*ti))
				return false;
	return true;
}

void ECEntity::Played()
{
	/// @note this function is called by ECEntity::Return()
	ECBEntity::Played();
	Move()->Clear(Case());
	Events()->Clear();
}

ECEntity::~ECEntity()
{

}

void ECEntity::Shoot(ECEntity* e, uint k)
{
	if(Owner())
		dynamic_cast<ECPlayer*>(Owner())->Stats()->shooted += k;

	Debug(W_DEBUG, "%s shoot %s de %d", LongName().c_str(), e->LongName().c_str(), k);
	e->Shooted(k);
}

void ECEntity::ReleaseShoot()
{
	SetNb(Nb() - (shooted > Nb() ? Nb() : shooted));
	if(Owner())
		Owner()->Stats()->killed += shooted;
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

bool ECEntity::Return()
{
	// On en revient à l'état après une partie.
	Played();

	return true;
}

bool ECEntity::Attaq(std::vector<ECEntity*> entities, ECEvent*)
{
	uint enemies = 0;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(this != *it && (*it)->Case() == Case() && !Like(*it) && (*it)->Nb() &&
		   (CanAttaq(*it) || Parent() && Parent()->CanAttaq(*it)))
			enemies++;

	if(!enemies) return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && (*it)->Case() == Case() && !Like(*it) && (CanAttaq(*it) || Parent() && Parent()->CanAttaq(*it)))
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

void ECEntity::CancelEvents(bool send_message)
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

	Events()->Clear();
	Return();

	if(send_message && Owner() && Owner()->Client())
		Channel()->SendArm(Owner()->Client(), this, ARM_RETURN, Case()->X(), Case()->Y());

	// On annule les evenements des unités liées à ces evenements
	FORit(ECEntity*, entities, enti)
		(*enti)->CancelEvents(true); // true signifie qu'on va envoyer un ARM_RETURN
}
