/* src/Timer.cpp - Make a delayed timer - By ClanBomber.
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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
 * $Id$
 */

#include <ClanLib/Core/System/system.h>
#include "Timer.h"

int Timer::begin_time = 0;
float Timer::elapsed = 0;

Timer::Timer()
{
	reset();
}

Timer::~Timer()
{
}

void Timer::reset()
{
	begin_time = CL_System::get_time();
	elapsed = 0;
}

float Timer::time_elapsed(bool update)
{
	if (update)
	{
		elapsed = (CL_System::get_time() - begin_time)/1000.0f;
		begin_time = CL_System::get_time();
	}

	return elapsed;
}





