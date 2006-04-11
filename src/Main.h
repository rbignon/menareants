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
#include "tools/Font.h"
#include "gui/Object.h"

#include <iostream>
#include <SDL_thread.h>

struct SDL_Surface;
class TForm;
class EChannel;
class EC_Client;

class MenAreAntsApp
{
protected:
	
	EC_Client* client;
	Config *conf;
	SDL_Thread* Thread;
	Fonts *fonts;
	std::string path;
	bool want_quit;

	void request_game();
	void ListGames();
	bool GameInfos(const char* c, TForm* f = 0);
	void LoadGame(EChannel* ch);
	void InGame();

	static void WantQuit(TObject*, void*);
	static void WantPlay(TObject*, void*);
	static void WantConfig(TObject*, void*);

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
	Fonts* Font() const { return fonts; }
	void setclient(EC_Client* c);
	std::string GetPath() const { return path; }

	MenAreAntsApp() {
		client = 0;
		conf = 0;
		Thread = 0;
		fonts = 0;
		want_quit = false;
	}

	SDL_Surface* sdlwindow;
};

extern MenAreAntsApp app;

#endif
