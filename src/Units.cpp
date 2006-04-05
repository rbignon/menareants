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

#define ARMY_VISUAL_STEP 3
bool ECArmy::MakeEvent()
{
	ECMap* map = dynamic_cast<ECMap*>(acase->Map());
	switch(event_type)
	{
		case ARM_ATTAQ:
		case ARM_MOVE:
		case ARM_ATTAQ|ARM_MOVE:
		{
			if(move.Empty())
				return true;

			ECMove::E_Move m = move.First();
			switch(m)
			{
				case ECMove::Right: image->set(image->X() + ARMY_VISUAL_STEP, image->Y()); break;
				case ECMove::Left:  image->set(image->X() - ARMY_VISUAL_STEP, image->Y()); break;
				case ECMove::Down:  image->set(image->X(), image->Y() + ARMY_VISUAL_STEP); break;
				case ECMove::Up:    image->set(image->X(), image->Y() - ARMY_VISUAL_STEP); break;
			}
			SDL_Delay(50);
			switch(m)
			{
				case ECMove::Right:
					if(map->ShowMap()->X() + (CASE_WIDTH * int(acase->X()+1)) <= image->X())
						ChangeCase(acase->MoveRight()), move.RemoveFirst();
					break;
				case ECMove::Left:
					if(map->ShowMap()->X() + (CASE_WIDTH * int(acase->X()-1)) >= image->X())
						ChangeCase(acase->MoveLeft()), move.RemoveFirst();
					break;
				case ECMove::Down:
					if(map->ShowMap()->Y() + (CASE_HEIGHT * int(acase->Y()+1)) <= image->Y())
						ChangeCase(acase->MoveDown()), move.RemoveFirst();
					break;
				case ECMove::Up:
					if(map->ShowMap()->Y() + (CASE_HEIGHT * int(acase->Y()-1)) >= image->Y())
						ChangeCase(acase->MoveUp()), move.RemoveFirst();
					break;
			}

			printf("- %d,%d\n", image->X(), image->Y());
			return false;
			break;
		}
		default: break;
	}
	printf("? %d (%s)\n", event_type, LongName().c_str());
	return true;
}

bool ECArmy::AfterEvent()
{
	return true;
}
