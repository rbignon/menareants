/* src/main.h - Main file
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
 * $Id$
 */

#ifndef EC_Main_h
#define EC_Main_h

#include "Defines.h"
#include "Config.h"
#include "Sockets.h"

#include <iostream>
#include <SDL_thread.h>

struct SDL_Surface;

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
	OPTIONS_TEST,
	OPTIONS_RETOUR
};

class EuroConqApp
{
protected:
	Menu* menu;
	EC_Client* client;
	Config *conf;
	SDL_Thread* Thread;
	std::string path;

	void request_game();
	void ListGames();
	bool GameInfos(const char* c);

public:
	int main(int argc, char** argv);
	char *get_title() { return APP_NAME; }
#ifndef WIN32
	void quit_app(int value) __attribute__ ((noreturn));
#else
	void quit_app(int value);
#endif

	Config* getconf() const { return conf; }
	EC_Client* getclient() const { return client; }
	void setclient(EC_Client* c);
	Menu* getmenu() const { return menu; }
	std::string GetPath() const { return path; }

	EuroConqApp() {
		menu = 0;
		client = 0;
		conf = 0;
		Thread = 0;
	}

	SDL_Surface* sdlwindow;
};

extern EuroConqApp app;

#endif
