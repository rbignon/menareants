/* src/main.h - Main file
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

#ifndef EC_Main_h
#define EC_Main_h

#include "Resources.h"

#include "Defines.h"
#include "Config.h"
#include "Sockets.h"

#include <iostream>

#ifndef WIN32
	#include <config.h>
#endif


class Menu;

class EC_Client;

enum {
	MENU_JOUER,
	MENU_OPTIONS,
	MENU_EXIT,

	JOUER_LISTER,
	JOUER_CREER,
	JOUER_RETOUR,

	OPTIONS_HOST,
	OPTIONS_PORT,
	OPTIONS_NICK,
	OPTIONS_RETOUR
};

class EuroConqApp
{
protected:
	Menu* menu;
	EC_Client* client;
	Config *conf;

public:
	int main(int argc, char** argv);
	char *get_title() { return APP_NAME; }
#ifndef WIN32
	void quit_app(int value) __attribute__ ((noreturn));
#else
	void quit_app(int value);
#endif

	Config* getconf() { return conf; }
	EC_Client* getclient() { return client; }
	Menu* getmenu() { return menu; }

	static bool connect_to_server(bool);

	SDL_Surface* sdlwindow;
};

extern EuroConqApp app;

#endif
