/* test/map.cpp - This program is here to check if a map file is ok.
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

#include "Map.h"
#include "Debug.h"
#include <iostream>
#include <string>

#define DEFAULT_FILE "gna.map"

int main(int argc, char **argv)
{
	char* filename = 0;
	if(argc > 1)
		filename = argv[1];
	else
		filename = DEFAULT_FILE;

	ECBMap *map = 0;
	try
	{
		map = new ECBMap("../server/maps/" + std::string(filename));
		map->Init();

		/* Utilisation de printf parce que c'est plus beau */
		printf("Map \"%s\" is loaded !!\n", map->Name().c_str());
		printf("-----------------------\n");
		printf("IniDate:     %10s\n", map->Date()->String().c_str());
		++(*map->Date());
		printf("NextDate:    %10s\n", map->Date()->String().c_str());
		printf("NbPlayers:   %10d\n", map->NbMapPlayers());
		printf("NbCases:     %10d\n", map->NbCases());
		printf("NbCountries: %10d\n", map->NbCountries());
		printf("MinPlayers:  %10d\n", map->MinPlayers());
		printf("MaxPlayers:  %10d\n", map->MaxPlayers());
		printf("BeginFric:   %10d\n", map->BeginMoney());
		printf("CityFric:    %10d\n", map->CityMoney());
		printf("-----------------------\n");
		printf("TERRAIN :\n\n");
		for(unsigned y = 0; y < map->Height(); y++)
		{
			for(unsigned x = 0; x < map->Width(); x++)
				printf("[%c]", (*map)(x, y)->TypeID());
			printf("\n");
		}
		printf("-----------------------\n");
		printf("COUNTRIES :\n\n");
		for(unsigned y = 0; y < map->Height(); y++)
		{
			for(unsigned x = 0; x < map->Width(); x++)
				printf("|%s", (*map)(x, y)->Country()->ID());
			printf("|\n");
		}
		printf("-----------------------\n");
		printf("PLAYERS :\n\n");
		for(unsigned y = 0; y < map->Height(); y++)
		{
			for(unsigned x = 0; x < map->Width(); x++)
				printf("[%c]", (*map)(x, y)->Country()->Owner() ? (*map)(x, y)->Country()->Owner()->ID() : '*');
			printf("\n");
		}
		printf("-----------------------\n");
		std::vector<ECBMapPlayer*> m_p = map->MapPlayers();
		for(std::vector<ECBMapPlayer*>::iterator it=m_p.begin(); it != m_p.end(); ++it)
			printf("Player %c : %d countries\n", (*it)->ID(), (*it)->Countries().size());
	}
	catch(TECExcept &e)
	{
		std::cout << "Erreur    : " << e.Message << std::endl;
		std::cout << "Variables : " << e.Vars << std::endl;
	}

	delete map;
	return 0;
}
