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
#include <fstream>
#include <algorithm>

MapVector MapList;

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

bool ECEntity::AreFriends(std::vector<ECEntity*> list)
{
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
		for(std::vector<ECEntity*>::iterator ti = list.begin(); ti != list.end(); ++ti)
			if(!(*ti)->Like(*it) || !(*it)->Like(*ti))
				return false;
	return true;
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
