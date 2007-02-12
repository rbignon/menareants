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
	std::ifstream fp(MAP_FILE);

	if(!fp)
	{
		if(!(app.HasFlag(ECServer::F_SILENT)))
			Debug(W_ERR|W_ECHO, "Unable to load map list files: %s", MAP_FILE);
		return false;
	}

	std::string ligne;
	uint nbmaps = 0, nbmissions = 0;

	while(std::getline(fp, ligne))
	{
		if(ligne[0] == '#' || ligne[0] == '\0') continue;
		ECMap *map = 0;
		try
		{
			map = new ECMap(std::string(PKGDATADIR + ligne));
			map->Init();
		}
		catch(TECExcept &e)
		{
			delete map;
			if(!(app.HasFlag(ECServer::F_SILENT)))
			{
				Debug(W_ERR|W_ECHO, "Unable to load this map file : %s", ligne.c_str());
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
	if(!Channel()) throw ECExcept(VPName(Channel()), "Pas de channel li� !?");

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
		{ /* On sait jamais, si par exemple une unit� peut en investir une autre, mais que c'est cette autre qui se deplace
		   * vers la mienne, ou si les deux se d�placent dans le m�me tour sur la meme case, �a serait con que juste
		   * parce que l'unit� investisseuse s'est d�plac�e *avant* l'unit� � investir, il n'y ait pas d'investiture
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
				if((*it)->IsCity())
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
	if(e->IsCountryMaker())
		Case()->Country()->ChangeOwner(Owner()->MapPlayer());
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

	/* On change de propri�taire les evenements */
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
	SetNb(Nb() - (shooted > nb ? nb : shooted));
	if(Owner())
		Owner()->Stats()->killed += shooted;
	shooted = 0;
}

ECPlayer* ECEntity::Owner() const
{
	return dynamic_cast<ECPlayer*>(ECBEntity::Owner());
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

	/* On lock car il fait maintenant partie int�grale de (*enti) */
	entity->SetZombie();

	/* On met dans le nouvel etat de l'entit� le nouveau nombre de soldats */
	SetNb(Nb() + entity->Nb());
	/* Enfin on d�fini le nombre de pas restants */
	SetRestStep(RestStep() > (entity->MyStep() - entity->RestStep()) ? RestStep() - (entity->MyStep() - entity->RestStep())
	                                                                 : 0);
}

bool ECEntity::Return()
{
	// On en revient � l'�tat apr�s une partie.
	ECEntity::Played();

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
			uint killed = rand() % (nb/2+enemies);
			if(killed < nb/(4+enemies)) killed = nb/(4+enemies);
			Shoot(*it, killed);
		}

	return true;
}

std::nrvector<TClient*> ECEntity::EntitiesToClients(std::vector<ECEntity*> entities)
{
	std::nrvector<TClient*> players;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if((*it)->Owner() && (*it)->Owner()->Client())
			players.push_back((*it)->Owner()->Client());
	return players;
}

void ECEntity::CancelEvents()
{
	ECMap* map = dynamic_cast<ECMap*>(Map());
	EventVector events = Events()->List();
	for(EventVector::iterator evti = events.begin(); evti != events.end(); ++evti)
	{
		if(Owner())
			Owner()->Events()->Remove(*evti);
		map->RemoveEvent(*evti, USE_DELETE);
	}

	Events()->Clear();
	Return();
}
