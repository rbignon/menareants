/* src/AltThread.cpp - This is a thread who can be used to make some long effects (in time)
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

#include "AltThread.h"
#include <SDL_thread.h>
#include <SDL.h>

bool ECAltThread::want_quit = false;
bool ECAltThread::running = false;
std::vector<ECAltFunction> ECAltThread::functions;
std::vector<void*> ECAltThread::args;

int ECAltThread::Exec(void *data)
{
	if(running) return 0;

	running = true;
	while(!want_quit)
	{
		if(!functions.empty())
		{
			SDL_LockMutex((SDL_mutex*)data);
			for(std::vector<ECAltFunction>::iterator it = functions.begin(); it != functions.end();)
			{
				if(args.empty()) continue;
				(**it) (*(args.begin()));
				it = functions.erase(it);
				args.erase(args.begin());
			}
			SDL_UnlockMutex((SDL_mutex*)data);
		}
		SDL_Delay(10);
	}

	running = false;
	return 0;
}
