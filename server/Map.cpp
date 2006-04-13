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
		if((*it)->Case() == c && (*it)->Flags() == f && (!e || (*it)->Entities()->Find(e)))
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
	const char UNION = 7;
	const char ATTAQ = 5;
	const char MOVE = 2;
	char me = 0, him = 0;
	switch(flags)
	{
		case ARM_NUMBER: me = NUMBER; break;
		case ARM_CREATE: me = CREATE; break;
		case ARM_UNION: me = UNION; break;
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
			if(!(*ti)->Like(*it) || !(*it)->Like(*ti))
				return false;
	return true;
}

void ECEntity::AddUnits(uint u)
{
	CreateLast();
	ECBEntity::AddUnits(u);
}

void ECEntity::Played()
{
	ECBEntity::Played();
	RemoveLast();
}

ECEntity::~ECEntity()
{
	RemoveLast();
}

void ECEntity::RemoveLast()
{
	if(!last) return;

	last->RemoveLast();
	last->Case()->Entities()->Remove(last);
	MyFree(last);
}

ECEntity* ECEntity::FindLast(ECBCase* c)
{
	for(ECEntity* a_l = this; a_l; a_l = a_l->Last())
	{
		printf("(%p(%d,%d) == %p(%d,%d))\n", a_l->Case(), a_l->Case()->X(), a_l->Case()->Y(), c, c->X(), c->Y());
		if(a_l->Case() == c)
			return a_l;
	}
	return 0;
}
