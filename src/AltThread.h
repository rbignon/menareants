/* src/AltThread.h - Header of AltThread.cpp
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

#ifndef EC_ALTTHREAD_H
#define EC_ALTTHREAD_H

#include <stack>

struct SDL_mutex;

typedef void (*ECAltFunction) (void* Data1);

class ECAltThread
{
/* Constructeur/Destructeur */
private:

	ECAltThread() {}
	ECAltThread(ECAltThread&) {}

/* Methodes */
public:

	static int Exec(void *data);

	static void LockThread();
	static void UnlockThread();

/* Attributs */
public:

	static void Put(ECAltFunction f, void* a) { functions.push(f); args.push(a); }

	static void Stop() { want_quit = true; }

/* Variables privées */
private:
	typedef std::stack<ECAltFunction> alt_list;
	static alt_list functions;
	static std::stack<void*> args;
	static bool want_quit;
	static bool running;
	static SDL_mutex* mutex;
};

#endif /* EC_ALTTHREAD_H */
