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

#include <stddef.h>
#include "Timer.h"

#ifdef WIN32
#  include <time.h>
#  include <windows.h>
#  ifndef _TIMEVAL_DEFINED /* also in winsock[2].h */
#    define _TIMEVAL_DEFINED
struct timeval {
    long tv_sec;
    long tv_usec;
};
#  endif /* _TIMEVAL_DEFINED */
#else
#  include <sys/time.h>
#endif

#ifdef WIN32
int gettimeofday(struct timeval* tp, void* tzp) {
    DWORD t;
    t = timeGetTime();
    tp->tv_sec = t / 1000;
    tp->tv_usec = t % 1000;
    /* 0 indicates that the call succeeded. */
    return 0;
}
#endif

Timer::Timer()
{
	reset();
}

Timer::~Timer()
{
}

void Timer::reset()
{
	begin_time = Timer::get_time();
	elapsed = 0;
	pause = 0;
}

bool Timer::Pause(bool set)
{
	if(pause && !set)
	{
		begin_time = begin_time + get_time() - pause;
		pause = 0;
		return false;
	}
	else if(!pause && set)
	{
		pause = get_time();
		return true;
	}
	return pause;
}

float Timer::time_elapsed(bool update)
{
	if (update)
		elapsed = float(get_time() - begin_time)/1000.0f;

	return pause ? pause : elapsed;
}

long Timer::get_time()
{
        struct timeval tv;

        gettimeofday( &tv, NULL );

        return (tv.tv_sec*1000  +  tv.tv_usec/1000);
}
