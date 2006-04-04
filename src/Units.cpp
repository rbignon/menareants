/* src/Units.cpp - Code of units
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
#include "gui/ShowMap.h"

bool ECArmy::BeforeEvent()
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
			SetAnim(true);
			break;
		default: break;
	}
	return true;
}

bool ECArmy::MakeEvent()
{
#if 0
	ECMap* map = dynamic_cast<ECMap*>(acase->Map());
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
		{
			if(!new_case || new_case == Case())
				return true;

			if(Case()->X() != new_case->X())
			{
				if(Case()->X() < new_case->X() &&
				   (map->ShowMap()->X() + (CASE_WIDTH  * int(acase->MoveRight()->X()))) <= image->X())
					ChangeCase(acase->MoveRight());
				else if(Case()->X() > new_case->X() &&
				        (map->ShowMap()->X() + (CASE_WIDTH  * int(acase->MoveLeft()->X()))) >= image->X())
					ChangeCase(acase->MoveLeft());
				else
				{
					image->set(image->X() + (new_case->X() > Case()->X() ? 3 : -3), image->Y());
					SDL_Delay(50);
				}
			}
			else if(Case()->Y() != new_case->Y())
			{
				if(Case()->Y() < new_case->Y() &&
				   (map->ShowMap()->Y() + (CASE_HEIGHT  * int(acase->MoveDown()->Y()))) <= image->Y())
					ChangeCase(acase->MoveDown());
				else if(Case()->Y() > new_case->Y() &&
				        (map->ShowMap()->Y() + (CASE_HEIGHT  * int(acase->MoveUp()->Y()))) >= image->Y())
					ChangeCase(acase->MoveUp());
				else
				{
					image->set(image->X(), image->Y() + (new_case->Y() > Case()->Y() ? 3 : -3));
					SDL_Delay(50);
				}
			}
			printf("- %d,%d\n", image->X(), image->Y());
			if(new_case == Case()) return true;
			else return false;
			break;
		}
		default: break;
	}
	printf("? %d (%s)\n", event_type, ID());
#endif
	return true;
}

bool ECArmy::AfterEvent()
{
	return true;
}
