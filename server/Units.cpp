/* server/Units.cpp - Units in server
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

#include "Units.h"
#include "Channels.h"

bool ECArmy::Attaq(std::vector<ECEntity*> entities)
{
	uint enemies = 0;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			if(this != *it && !(*it)->Locked() && (!(*it)->Like(this) || !Like(*it)) &&
			                                   ((*it)->CanAttaq(this) || CanAttaq(*it)))
				enemies++;

	if(!enemies) return true;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !(*it)->Locked() && !Like(*it) && CanAttaq(*it))
		{
			uint killed = rand() % (nb/2+enemies);
			if(killed < nb/(4+enemies)) killed = nb/(4+enemies);
			(*it)->Shooted(killed);
			printf("%s!%s shoot %s!%s de %d\n", Owner() ? Owner()->GetNick() : "*", ID(),
			                                     (*it)->Owner() ? (*it)->Owner()->GetNick() : "*", (*it)->ID(),
			                                     killed);
		}

	return true;
}
