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
#include <fstream>
#include <algorithm>

MapVector MapList;

/********************************************************************************************
 *                                 ECMap                                                    *
 ********************************************************************************************/
 
bool LoadMaps()
{
	std::ifstream fp(MAP_FILE);

	if(!fp)
	{
		Debug(W_ERR|W_ECHO, "Unable to load map list files: %s", MAP_FILE);
		return false;
	}

	std::string ligne;
	uint nbmaps = 0;

	while(std::getline(fp, ligne))
	{
		if(ligne[0] == '#' || ligne[0] == '\0') continue;
		ECMap *map = 0;
		try
		{
			map = new ECMap(std::string(PKGDATADIR + ligne), nbmaps);
			map->Init();
		}
		catch(TECExcept &e)
		{
			delete map;
			Debug(W_ERR|W_ECHO, "Unable to load this map file : %s", ligne.c_str());
			vDebug(W_ERR|W_ECHO, e.Message, e.Vars);
			continue;
		}
		nbmaps++;
		MapList.push_back(map);
	}
	Debug(W_ECHO|W_NOLOG, "%d maps loaded !", nbmaps);
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

ECEvent* ECMap::FindEvent(ECase* c, uint f, ECEntity* e)
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end(); ++it)
		if((!c || (*it)->Case() == c) && (*it)->Flags() == f && (!e || (*it)->Entities()->Find(e)))
			return *it;
	return 0;
}

struct SortEventsFunction
{
	bool operator ()(ECEvent* const& a1, ECEvent* const& a2) const
	{
		return (*a2) < (*a1);
	}
};
void ECMap::SortEvents()
{
	std::sort(map_events.begin(), map_events.end(), SortEventsFunction());
}

/********************************************************************************************
 *                                 ECountry                                                 *
 ********************************************************************************************/

bool ECountry::ChangeOwner(ECBMapPlayer* mp)
{
	ECBMapPlayer* last_owner = owner;
	if(ECBCountry::ChangeOwner(mp))
	{
		if(owner && owner->Player())
		{
			EChannel* chan = dynamic_cast<EChannel*>(owner->Player()->Channel());
			chan->sendto_players(0, app.rpl(ECServer::SET), owner->Player()->GetNick(),
									std::string(std::string("+@ ") + ident).c_str());
		}
		else if(last_owner && last_owner->Player())
		{
			EChannel* chan = dynamic_cast<EChannel*>(last_owner->Player()->Channel());
			chan->sendto_players(0, app.rpl(ECServer::SET), last_owner->Player()->GetNick(),
									std::string(std::string("-@ ") + ident).c_str());
		}
		return true;
	}
	return false;
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

bool ECEvent::operator<(const ECEvent& e) const
{
	const char NUMBER = 10;
	const char CREATE = 9;
	const char MOVE   = 8;
	const char UNION  = 7;
	const char DEPLOY = 6;
	const char ATTAQ  = 5;
	char me = 0, him = 0;
	switch(flags)
	{
		case ARM_NUMBER: me = NUMBER; break;
		case ARM_CREATE: me = CREATE; break;
		case ARM_UNION: me = UNION; break;
		case ARM_DEPLOY: me = DEPLOY; break;
		case ARM_ATTAQ: me = ATTAQ; break;
		case ARM_MOVE: me = MOVE; break;
		default: me = 0; break;
	}
	switch(e.Flags())
	{
		case ARM_NUMBER: him = NUMBER; break;
		case ARM_CREATE: him = CREATE; break;
		case ARM_UNION: him = UNION; break;
		case ARM_ATTAQ: him = ATTAQ; break;
		case ARM_DEPLOY: him = DEPLOY; break;
		case ARM_MOVE: him = MOVE; break;
		default: him = 0; break;
	}
	return (me < him);
}

bool ECEvent::CheckRemoveBecauseOfPartOfAttaqEntity(ECEntity* entity)
{
	/* Mauvais appel */
	if(flags != ARM_ATTAQ) return false;

	EChannel* chan = dynamic_cast<EChannel*>(entity->Owner()->Channel());
	ECMap* map = dynamic_cast<ECMap*>(chan->Map());

	bool delete_event = true;
	Debug(W_DEBUG, "Retrait d'attaquants");
	Entities()->Remove(entity);
	std::vector<ECEntity*> ents = Entities()->List();

	const char T_ATTAQ_STEP = 0;
	const char T_MOVE_STEP = 1;
	const char T_END_STEP = 2;
	for(char step = T_ATTAQ_STEP; step != T_END_STEP; ++step)
		for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end();)
		{
			if((*enti)->Locked())
			{
				++enti;
				continue;
			}

			/* Pour cette entité, on cherche si il peut encore attaquer quelqu'un d'autre, sinon il jarte */
			std::vector<ECEntity*>::iterator enti2;
			for(enti2 = ents.begin();
				enti2 != ents.end() && (*enti2 == *enti || (*enti2)->Locked() ||
					(!(*enti)->CanAttaq(*enti2) || (*enti)->Like(*enti2)) &&
					(step != T_MOVE_STEP || !(*enti2)->CanAttaq(*enti) || (*enti2)->Like(*enti)));
				++enti2);
			if(enti2 == ents.end())
			{
				if(step == T_ATTAQ_STEP && (*enti)->EventType() == ARM_ATTAQ)
				{ /* C'était un attaquant, on lui fait faire un Return */
					Debug(W_DEBUG, "il y a un attaquant qui se trouve fort sodomisé.");
					if((*enti)->Return() && (*enti)->Owner())
						chan->SendArm(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client(), *enti,
										ARM_RETURN, (*enti)->Case()->X(), (*enti)->Case()->Y());

					/* On supprime l'evenement de mouvement linké */
					std::vector<ECEvent*>::iterator ev;
					for(ev = this->linked.begin(); ev != this->linked.end() && (*ev)->Entity() != *enti; ++ev);
					if(ev != this->linked.end())
					{
						delete *ev;
						ev = this->linked.erase(ev);
					}
				}
				else if(step == T_MOVE_STEP && (*enti)->EventType() == ARM_MOVE)
				{ /* C'était un mouvement innocent, on remet juste son evenement en lieu commun */
					std::vector<ECEvent*>::iterator ev;
					for(ev = this->linked.begin(); ev != this->linked.end() && (*ev)->Entity() != *enti; ++ev);
					if(ev != this->linked.end())
					{
						map->AddEvent(*ev);
						ev = this->linked.erase(ev);
					}
				}
				else
				{
					++enti;
					continue;
				}
				this->Entities()->Remove(*enti);
				enti = ents.erase(enti);
			}
			else
			{
				/* Cet evenement réuni encore des unités qui peuvent se battre */
				delete_event = false;
				++enti;
			}
		}
#ifdef DEBUG
	if(delete_event)
		Debug(W_DEBUG, "On supprime bien l'evenement");
	else
		Debug(W_DEBUG, "On ne supprime pas, doit y avoir d'autres unités combatantes");
#endif
	return delete_event;
}

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/

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
	ECBEntity::Played();
	RemoveLast();
	Move()->Clear(Case());
}

ECEntity::~ECEntity()
{
	RemoveLast();
}

EChannel* ECEntity::Channel() const
{
	assert(Case());
	assert(Case()->Map());
	assert(Case()->Map()->Channel());
	return dynamic_cast<EChannel*>(Case()->Map()->Channel());
}

void ECEntity::RemoveLast()
{
	if(!last) return;

	//last->RemoveLast();
	last->Case()->Entities()->Remove(last);
	last->Case()->Map()->Entities()->Remove(last);

	// Le destructeur va appeler le RemoveLast() de l'entité last
	MyFree(last);
}

ECEntity* ECEntity::FindLast(ECBCase* c)
{
	for(ECEntity* a_l = this; a_l; a_l = a_l->Last())
		if(a_l->Case() == c)
			return a_l;
	return 0;
}

ECEntity* ECEntity::FindNext()
{
	ECEntity* a_l = this;
	for(; a_l->Next(); a_l = a_l->Next());
	return a_l;
}

void ECEntity::Union(ECEntity* entity)
{
	if(entity->Type() != Type()) throw ECExcept(VIName(entity->Type()) VIName(Type()), "Union avec un autre type !?");

	/* On lock car il fait maintenant partie intégrale de (*enti) */
	entity->SetLock();
	/* On créé un clone de l'ancienne entité. */
	CreateLast();
	/* On met dans le nouvel etat de l'entité le nouveau nombre de soldats */
	SetNb(Nb() + entity->Nb());
	/* Enfin on défini le nombre de pas restants */
	SetRestStep(RestStep() > entity->RestStep() ? RestStep() - entity->RestStep() : 0);
}

bool ECEntity::Return(ECBCase* c)
{
	printf("engoument.... (%d,%d)\n", acase->X(), acase->Y());

	/* Si il n'y a pas de last on a rien à faire */
	if(!last) return false;

	/* La récurence de Return() s'arrête jusqu'à un changement d'etat
	 * autre que la position, ou si, dans le cas d'une case spécifiée,
	 * on a atteint le last qui est sur cette case.
	 */
	if(!c && last->Case() == Case() || c == Case()) return false;

	/* On appel le Return() du last */
	last->Return(c);
	printf("on sort d'un engouement (%d,%d)\n", last->Case()->X(), last->Case()->Y());

	ECEntity* lastlast = last;

	if(last)
	{
		move = *(last->Move());
	}

	/* On prend comme last le last de notre last */
	last = lastlast->Last();
	lastlast->last = 0;

	printf("last = (%d,%d)\n", last ? last->Case()->X() : 0, last ? last->Case()->Y():0);

	/* On récupère les pas */
	if(last && last->Case() != acase)
		restStep = !last ? myStep : (last->RestStep()) ? last->RestStep() - 1 : 0;

	if(last)
		last->next = this;

	/* On s'enlève de la case où on est actuellement et on se met sur l'ancienne case */
	acase->Entities()->Remove(this);
	printf("on se vire de (%d,%d)\n", acase->X(), acase->Y());
	acase = lastlast->Case();
	acase->Entities()->Add(this);
	printf("on s'ajoute sur (%d,%d)\n", acase->X(), acase->Y());

	/* On enlève le last de la case */
	lastlast->Case()->Entities()->Remove(lastlast);
	printf("on vire un last de (%d,%d)\n", lastlast->Case()->X(), lastlast->Case()->Y());

	/* On supprime le last */
	MyFree(lastlast);

	return true;
}

bool ECEntity::Attaq(std::vector<ECEntity*> entities)
{
	uint enemies = 0;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(this != *it && (*it)->Case() == Case() && !Like(*it) && CanAttaq(*it))
			enemies++;

	if(!enemies) return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && (*it)->Case() == Case() && !Like(*it) && CanAttaq(*it))
		{
			uint killed = rand() % (nb/2+enemies);
			if(killed < nb/(4+enemies)) killed = nb/(4+enemies);
			(*it)->Shooted(killed);
			Debug(W_DEBUG, "%s shoot %s de %d", LongName().c_str(), (*it)->LongName().c_str(), killed);
		}

	return true;
}
