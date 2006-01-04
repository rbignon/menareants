/* src/Config.cpp - Configuration
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

Config::Config(std::string _filename)
{
	filename = _filename;
}

bool Config::set_defaults()
{
	hostname = "127.0.0.1";
	port = 6667;
	nick = "anonyme";
	return true;
}

bool Config::load()
{
	std::ifstream fp(filename.c_str());

	if(!fp)
		return set_defaults();

	std::string ligne;

	while(std::getline(fp, ligne))
	{
		std::string key = stringtok(ligne, " ");

		if(key == "SERVER") hostname = ligne;
		else if(key == "PORT") port = atoi(ligne.c_str());
		else if(key == "NICK") nick = ligne;
		else
		{
			std::cout << "Fichier incorrect" << std::endl;
			return set_defaults();
		}
	}
	if(port < 1 || port > 65535)
	{
		std::cout << "Lecture de la configuration invalide : " << port << std::endl;
		return set_defaults();
	}
	return true;
}

bool Config::save() const
{
    std::ofstream fp(filename.c_str());
    if (!fp)
    {
        std::cerr << "Impossible de créer le fichier de configuration" << std::endl;
        return 0;
    }

    fp << "SERVER " << hostname << std::endl;
    fp << "PORT " << port << std::endl;
    fp << "NICK " << nick << std::endl;

	return true;
}
