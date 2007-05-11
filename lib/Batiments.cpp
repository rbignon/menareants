/* lib/Batiments.cpp - Base structures of batiments in game.
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

#include "Batiments.h"
#include "Channels.h"
#include <assert.h>

/********************************************************************************************
 *                               ECBRadar                                                   *
 ********************************************************************************************/

bool ECBRadar::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	std::vector<ECBEntity*> ents = pl->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if(((*it)->Type() == E_RADAR || (*it)->Type() == E_EIFFELTOWER) &&
		   !((*it)->EventType() & ARM_SELL))
			return false;

	return true;
}

/********************************************************************************************
 *                               ECBNuclearSearch                                           *
 ********************************************************************************************/

void ECBNuclearSearch::Init()
{
	if(!Owner()) return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
	{
		ECBSilo* silo;
		if((silo = dynamic_cast<ECBSilo*>(*it)))
			silo->SetNuclearSearch(this);
	}
}

ECBNuclearSearch::~ECBNuclearSearch()
{
	if(!Owner()) return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
	{
		ECBSilo* silo;
		if((silo = dynamic_cast<ECBSilo*>(*it)))
			silo->SetNuclearSearch(0);
	}
}

bool ECBNuclearSearch::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	std::vector<ECBEntity*> ents = pl->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if(dynamic_cast<ECBNuclearSearch*>(*it) && !((*it)->EventType() && ARM_SELL))
			return false;

	return true;
}

/********************************************************************************************
 *                                    ECBSilo                                               *
 ********************************************************************************************/

void ECBSilo::Init()
{
	ECBEntity::Init();
	if(!Owner()) return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if((nuclear_search = dynamic_cast<ECBNuclearSearch*>(*it)))
			break;
}

bool ECBSilo::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	std::vector<ECBEntity*> ents = pl->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if(dynamic_cast<ECBNuclearSearch*>(*it) && !((*it)->EventType() && ARM_SELL))
			return true;

	return false;
}
