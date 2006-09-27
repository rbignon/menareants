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

#include "Outils.h"
#include "Config.h"
#include "Debug.h"

Config::Config(std::string _filename)
{
	filename = _filename;
}

bool Config::set_defaults()
{
	servername = "Men.Are.Ants";
	port = 5461;
	deflimite = 10;
	pingfreq = 50;
	maxconnexions = 10;
	maxgames = 3;
	// Ne *PAS* définir adminpass. Lors du premier chargement il sera nul si la conf est illisible, et donc
	// ça empechera le login en admin. Et dans le cas d'un REHASH foiré, le pass admin restera.
	return true;
}

bool Config::load()
{
	std::ifstream fp(filename.c_str());

	if(!fp)
		return false;

	set_defaults();

	std::string ligne;

	while(std::getline(fp, ligne))
	{
		if(ligne[0] == '#' || ligne.empty()) continue;
		std::string key = stringtok(ligne, " ");

		if(key == "SERVERNAME") servername = ligne;
		else if(key == "PORT")
		{
			port = StrToTyp<uint>(ligne);
			if(port < 1 || port > 65535)
			{
				std::cerr << "Le port donné est invalide" << std::endl;
				return false;
			}
		}
		else if(key == "DEFLIMITE")
		{
			deflimite = StrToTyp<uint>(ligne);
			if(deflimite < 2)
			{
				std::cerr << "La limite d'utilisateurs dans une partie doit être au moins à 2" << std::endl;
				return false;
			}
		}
		else if(key == "PINGFREQ") pingfreq = StrToTyp<uint>(ligne);
		else if(key == "MAXCONNEXIONS") maxconnexions = StrToTyp<uint>(ligne);
		else if(key == "MAXGAMES") maxgames = StrToTyp<uint>(ligne);
		else if(key == "MOTDFILE")
		{
			std::ifstream fp(ligne.c_str());

			if(fp)
			{
				std::string line;

				while(std::getline(fp, line))
					motd.push_back(line);
			}
			else
				Debug(W_WARNING|W_ECHO, "Le fichier de motd %s n'existe pas.", ligne.c_str());
		}
		else if(key == "ADMINPASS") adminpass = ligne;
		else
			Debug(W_WARNING|W_ECHO, "Ligne incorrecte (variable %s inconnue):\n%s %s",
			                        key.c_str(), key.c_str(), ligne.c_str());
	}

	return true;
}
