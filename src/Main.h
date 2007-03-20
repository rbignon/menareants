/* src/main.h - Main file
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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

//#include "Defines.h"
//#include "gui/Object.h"

#include <iostream>
#include <SDL_thread.h>

struct SDL_Surface;
class TForm;
class TObject;
class EChannel;
class EC_Client;

class MenAreAntsApp
{
private:

	SDL_mutex* mutex;
	std::string path;
	bool want_quit;

public:
	void ServerList();
	void RefreshList();
	void ConnectedTo(std::string name, std::string host);
	EC_Client* Connect(std::string host);
	void ListGames();
	#define G_MISSION      0x01
	#define G_ESCARMOUCHE  0x02
	bool GameInfos(const char* c, TForm* f = 0, int flags = 0);
	void LoadGame(EChannel* ch);
	void Options(EChannel* ch);
	void InGame();
	void MapEditor();
	void Scores(EChannel*);
	bool RecoverGame(std::string chaname);
	void PingingGame();

private:
	static void WantQuit(TObject*, void*);
	static void WantPlay(TObject*, void*);
	static void WantConfig(TObject*, void*);
	static void WantCredits(TObject*, void*);
	static void WantMapEditor(TObject*, void*);

	bool first_run;
	bool first_game;

public:
	int main(int argc, char** argv);
	char *get_title();

	void UnLoad();

	std::string GetPath() const { return path; }

	void FirstRun() { first_run = true; first_game = true; }
	bool IsFirstRun() const { return first_run; }
	void FirstRunDone() { first_run = false; }
	bool IsFirstGame() const { return first_game; }
	void FirstGameDone() { first_game = false; }

	SDL_mutex* Mutex() const { return mutex; }

	MenAreAntsApp()
		: mutex(0), want_quit(false), first_run(false), first_game(false)
	{}

	static MenAreAntsApp* singleton;
	static MenAreAntsApp* GetInstance();
};

#endif
