/* lib/Units.cpp - Units in game
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

#include "Units.h"
#include <assert.h>

/********************************************************************************************
 *                               ECBJouano                                                  *
 ********************************************************************************************/

bool ECBJouano::CanBeCreated(ECBPlayer* pl) const
{
	assert(pl);
	std::vector<ECBEntity*> ents = pl->Entities()->List();
	for(std::vector<ECBEntity*>::const_iterator it = ents.begin(); it != ents.end(); ++it)
		if((*it)->Type() == E_JOUANO)
			return false;

	return true;
}


/********************************************************************************************
 *                               ECBMcDo                                                    *
 ********************************************************************************************/
void ECBMcDo::Create(ECBEntity* e)
{
	if(!Deployed()) return;

	e->SetMyStep(e->Step()/2);
	e->SetRestStep(e->RestStep()/2);
}

/********************************************************************************************
 *                               ECBPlane                                                   *
 ********************************************************************************************/
int ECBPlane::TurnMoney(ECBPlayer* pl)
{
	if(Deployed() || !Containing() || Owner() != pl) return 0;

	return - (Containing()->Nb() * VolCost());
}
