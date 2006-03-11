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
			map = new ECMap(std::string(PKGDATADIR + ligne));
		}
		catch(TECExcept &e)
		{
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

void ECMap::AddAnEntity(ECBEntity* e)
{
	if(!e) return;

	e->Case()->Entities()->Add(e);
	if(e->Case()->Country()->Owner())
		e->Case()->Country()->Owner()->Player()->Entities()->Add(e);
	entities.Add(e);
}

void ECMap::RemoveAnEntity(ECBEntity* e, bool use_delete)
{
	if(!e) return;

	e->Case()->Entities()->Remove(e);
	entities.Remove(e);
	if(e->Owner())
		e->Owner()->Entities()->Remove(e);
	if(use_delete)
		delete e;
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
		return 0;
}
