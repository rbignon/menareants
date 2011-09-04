/* lib/Batiments.cpp - Base structures of batiments in game.
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

#include "Batiments.h"
#include "Channels.h"
#include <assert.h>

/********************************************************************************************
 *                               ECBCavern                                                  *
 ********************************************************************************************/
void ECBCavern::Init()
{
	ECBEntity::Init();

	SetContaining(NULL);
	if(!Owner()) return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
	{
		ECBCavern* cavern = dynamic_cast<ECBCavern*>(*it);
		if (!cavern || !cavern->Containing())
			continue;
		SetContaining(cavern->Containing());
	}
}

ECBCavern::~ECBCavern()
{
	UnContain();
}

void ECBCavern::SetOwner(ECBPlayer* p)
{
	ECBEntity::SetOwner(p);
	Init();
}

bool ECBCavern::Contain(ECBEntity* entity)
{
	if (!ECBContainer::Contain(entity))
		return false;

	BroadcastContaining(entity);

	return true;
}

bool ECBCavern::UnContain()
{
	if(!ECBContainer::UnContain())
		return false;

	BroadcastContaining(NULL);

	return true;
}

void ECBCavern::BroadcastContaining(ECBEntity* e)
{
	if (!Owner())
		return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
	{
		ECBCavern* cavern = dynamic_cast<ECBCavern*>(*it);
		if (!cavern)
			continue;
		cavern->SetContaining(e);
	}
}

/********************************************************************************************
 *                               ECBEiffelTower                                             *
 ********************************************************************************************/

bool ECBEiffelTower::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	if (pl->Nation() != ECBPlayer::N_FRANCE)
		return false;

	std::vector<ECBEntity*> ents = pl->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if(((*it)->Type() == E_RADAR || (*it)->Type() == E_EIFFELTOWER) &&
		   !((*it)->EventType() & ARM_SELL))
			return false;

	return true;
}

/********************************************************************************************
 *                               ECBRadar                                                   *
 ********************************************************************************************/

bool ECBRadar::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	if (pl->Nation() == ECBPlayer::N_FRANCE)
		return false;

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
	ECBEntity::Init();
	if(!Owner()) return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
	{
		ECBSilo* silo;
		if((silo = dynamic_cast<ECBSilo*>(*it)))
			silo->SetNuclearSearch(this);
	}
}

void ECBNuclearSearch::SetOwner(ECBPlayer* p)
{
	if (Owner())
	{
		std::vector<ECBEntity*> ents = Owner()->Entities()->List();
		for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		{
			ECBSilo* silo;
			if((silo = dynamic_cast<ECBSilo*>(*it)))
				silo->SetNuclearSearch(NULL);
		}
	}

	ECBEntity::SetOwner(p);

	Init();
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
		if(dynamic_cast<ECBNuclearSearch*>(*it) && !((*it)->EventType() & ARM_SELL))
			return false;

	return true;
}

/********************************************************************************************
 *                                    ECBSilo                                               *
 ********************************************************************************************/

void ECBSilo::Init()
{
	ECBEntity::Init();
	nuclear_search = NULL;
	if(!Owner())
		return;

	std::vector<ECBEntity*> ents = Owner()->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if((nuclear_search = dynamic_cast<ECBNuclearSearch*>(*it)))
			break;
}

void ECBSilo::SetOwner(ECBPlayer* p)
{
	ECBEntity::SetOwner(p);

	Init();
}

bool ECBSilo::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	std::vector<ECBEntity*> ents = pl->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if(dynamic_cast<ECBNuclearSearch*>(*it) && !((*it)->EventType() & ARM_SELL))
			return true;

	return false;
}
