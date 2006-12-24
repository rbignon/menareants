/* src/Config.h- Header of Config.cpp
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

#ifndef EC_Config_h
#define EC_Config_h

#include <string>
#include <vector>

class TObject;
class TListBox;

#define CLIENT_CONFVERSION 2

class Config
{
/* Constructeur/Destructeur */
public:
	Config();

	static Config* singleton;
	static Config* GetInstance();

/* Methodes */
public:
	bool load();
	bool save() const;
	void Configuration(bool);

	void SetFileName(std::string f) { filename = f; }

/* Variables publiques */
public:
	std::string hostname;
	int port;
	std::string nick;
	unsigned int color;
	unsigned int nation;
	unsigned int screen_width;
	unsigned int screen_height;
	std::vector<std::string> server_list;
	std::string ttf_file;
	bool fullscreen;
	bool music;
	bool effect;

/* Variable privées */
private:
	std::string filename;
	bool set_defaults(bool want_save = true);
	bool want_quit_config;

	static void WantCancel(TObject*, void*);
	static void WantOk(TObject*, void*);
	static void WantAddServer(TObject*, void*);
	static void WantDelServer(TObject*, void*);
	static void SetFullScreen(TObject*, void*);
	static void SetMusic(TObject*, void*);
	static void ChangeResolution(TListBox*);
};

#endif
