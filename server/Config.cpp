/* server/Config.cpp - Configuration
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

#include <string>
#include <fstream>
#include <iostream>

#include "lib/LibConfig.h"
#include "Outils.h"
#include "Config.h"
#include "Debug.h"

Config::Config(std::string _filename)
	: filename(_filename), conf(0)
{
}

Config::~Config()
{
	delete conf;
}

#if 0
bool Config::set_defaults()
{
	servername = "Men.Are.Ants";
	port = 5461;
	deflimite = 10;
	pingfreq = 50;
	maxconnexions = 10;
	maxgames = 3;
	motd.clear();
	// Ne *PAS* définir adminpass. Lors du premier chargement il sera nul si la conf est illisible, et donc
	// ça empechera le login en admin. Et dans le cas d'un REHASH foiré, le pass admin restera.
	return true;
}
#endif

std::string Config::ServerName() const { return conf->GetSection("server")->GetItem("name")->String(); }
uint Config::Port() const { return conf->GetSection("server")->GetItem("port")->Integer(); }
uint Config::DefLimite() const { return conf->GetSection("games")->GetItem("deflimite")->Integer(); }
uint Config::PingFreq() const { return conf->GetSection("server")->GetItem("fping")->Integer(); }
uint Config::MaxGames() const { return conf->GetSection("games")->GetItem("maxgames")->Integer(); }
uint Config::MaxConnexions() const { return conf->GetSection("server")->GetItem("maxcons")->Integer(); }
std::string Config::AdminPass() const { return conf->GetSection("server")->GetItem("adminpass")->String(); }
std::string Config::MSHost() const { return conf->GetSection("meta-server")->GetItem("host")->String(); }
uint Config::MSPort() const { return conf->GetSection("meta-server")->GetItem("port")->Integer(); }

bool Config::load()
{
	MyConfig* save = conf;
	conf = new MyConfig(filename);
	try
	{
		ConfigSection* section = conf->AddSection("server", "Declaration of server settings", false);
		section->AddItem(new ConfigItem_string("name", "Name of the server"), true);
		section->AddItem(new ConfigItem_int("port", "Port listened by server", 1, 65535));
		section->AddItem(new ConfigItem_int("fping", "Ping frequence", 0));
		section->AddItem(new ConfigItem_int("maxcons", "Maximum of connections", 0));
		section->AddItem(new ConfigItem_string("motdfile", "File for motd"));
		section->AddItem(new ConfigItem_string("adminpass", "Administrator password", " "));

		section = conf->AddSection("meta-server", "Meta-server settings", false);
		section->AddItem(new ConfigItem_string("host", "Hostname of meta-server"));
		section->AddItem(new ConfigItem_int("port", "Port of meta-server", 1, 65535));

		section = conf->AddSection("games", "Rules of games", false);
		section->AddItem(new ConfigItem_int("maxgames", "Maximum of games", 0));
		section->AddItem(new ConfigItem_int("deflimite", "Maximum of users in a game", 2));

		if(!conf->Load())
		{
			delete conf;
			conf = save;
			return false;
		}

		std::string motdfile = conf->GetSection("server")->GetItem("motdfile")->String();
		std::ifstream fp(motdfile.c_str());

		if(fp)
		{
			std::string line;

			while(std::getline(fp, line))
				motd.push_back(line);
		}
		else
			Debug(W_WARNING|W_ECHO, "Le fichier de motd %s n'existe pas.", motdfile.c_str());

		delete save;
		return true;
	}
	catch(MyConfig::error &e)
	{
		std::cerr << "Received an exception when initialisation of configuration :" << std::endl;
		std::cerr << " " << e.Reason() << std::endl;
	}
	delete conf;
	conf = save;
	return false;
}
