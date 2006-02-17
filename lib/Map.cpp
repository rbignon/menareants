/* lib/Map.cpp - Map classes
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

#include "Map.h"
#include "Debug.h"
#include "Outils.h"
#include <fstream>

/********************************************************************************************
 *                               ECBCase                                                    *
 ********************************************************************************************/

ECBCase::ECBCase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id)
	: map(_map),
	  x(_x),
	  y(_y),
	  flags(_flags),
	  type_id(_type_id)
{
	map_country = 0;
}

/********************************************************************************************
 *                               ECBCountry                                                 *
 ********************************************************************************************/

ECBCountry::ECBCountry(char _ident[3])
{
	strcpy(ident, _ident);
	owner = 0;
}

/********************************************************************************************
 *                               ECBMapPlayer                                               *
 ********************************************************************************************/

ECBCountry* ECBMapPlayer::FindCountry(const char* c)
{
	for(std::vector<ECBCountry*>::iterator it=countries.begin(); it != countries.end(); ++it)
		if(!strcmp(c, (*it)->ID()))
			return *it;
	return NULL;
}

/********************************************************************************************
 *                               ECBMap                                                     *
 ********************************************************************************************/

template<typename T>
static ECBCase* CreateCase(ECBMap *map, uint x, uint y, uint flags, char type_id)
{
	return new T(map, x, y, flags, type_id);
}

static struct
{
	char c;
	ECBCase* (*func) (ECBMap *map, uint x, uint y, uint flgs, char type_id);
	uint flags;
} case_type[] = {
	{ 'v', CreateCase<ECBVille>, C_VILLE            },
	{ 'V', CreateCase<ECBVille>, C_VILLE|C_CAPITALE },
	{ 'm', CreateCase<ECBMer>,   C_MER              },
	{ 't', CreateCase<ECBTerre>, C_TERRE            },
	{ 'p', CreateCase<ECBPont>,  C_PONT             }
};

ECBMap::ECBMap(std::vector<std::string> _map_file)
{
	initialised = false;

	map_file = _map_file;
	
	Init();
}

ECBMap::ECBMap(std::string filename)
{
	initialised = false;

	std::ifstream fp(filename.c_str());

	if(!fp)
		throw ECExcept(VName(filename), "Impossible d'ouvrir le fichier");

	std::string ligne;

	while(std::getline(fp, ligne))
		if(ligne[0] != '#' && ligne[0] != '\0')
			map_file.push_back(ligne);

	Init();
}

/** \page Map_Format Map's file format
 *
 * A map is defined by a file.
 *
 * This will respect this architecture :
 *
 * <pre>
 *   NAME [name]
 *   PLAYER [identificator]
 *   X [x]
 *   Y [y]
 *   MAP
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   EOM
 *   BEGIN [begining money]
 *   CITY [money by city]
 *   MIN [min]
 *   MAX [max]
 * </pre>
 *
 *  1) NAME [name]
 *   <pre>
 *   A map have a name. So put it here.
 *   </pre>
 *  2) PLAYER [identificator]
 *   <pre>
 *   It's a player. Identificator is an unik number ('A' to 'Z').
 *   </pre>
 *  3) X [x]
 *   <pre>
 *   This is the maximal number of horizontal cases.
 *   </pre>
 *  4) Y [y]
 *   <pre>
 *   This is the maximal number of vertical cases.
 *   </pre>
 *  5) MAP
 *   <pre>
 *   This is the body of map. After this message, and until have a "EOM" message,
 *   there is map body where identificators. All cases are identified like this :
 *
 *                       WXXYZ
 *
 *   W = Type of terrain (a city, sea)
 *      'v' = ville.
 *      'V' = ville | capitale.
 *      'm' = mer.
 *      't' = terre.
 *      'p' = pont.
 *   XX = Identificator of country (like AA, AB, etc)
 *   Y = Player Owner (like A, B, C, etc, or neutral)
 *   Z = Type of architecture (only for image, so not used by server)
 *   </pre>
 *
 *  6) BEGIN [begining money]
 *   <pre>
 *   Here we define money that all players have when they begin game.
 *   </pre>
 *
 *  7) CITY�[money by city]
 *   <pre>
 *   Here we put money win by each city.
 *   </pre>
 *
 *  8) MIN�[min]
 *   <pre>
 *   Min players.
 *   </pre>
 *
 *  6) MAX�[max]
 *   <pre>
 *   Max players. It's channel limit.
 *   </pre>
 *
 * @author Progs
 * @date 15/02/2006
 */
void ECBMap::Init()
{
	chan = 0;
	x = 0;
	y = 0;
	min = 0;
	max = 0;
	
	std::string ligne;

	bool recv_map = false;
	uint _x = 0, _y = 0;

	for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
	{
		ligne = (*it);
		if(ligne[0] == '#' || ligne[0] == '\0') continue; /* Commentaire */
		if(ligne == "EOM")
		{
			if(_y < y)
				throw ECExcept(VIName(_y) VIName(y), "Il n'y a pas assez de lignes !");
			recv_map = false;
			continue;
		}
		if(!recv_map)
		{
			std::string key = stringtok(ligne, " ");
	
			if(key == "NAME") name = ligne;
			else if(key == "X") x = atoi(ligne.c_str());
			else if(key == "Y") y = atoi(ligne.c_str());
			else if(key == "PLAYER")
				map_players.push_back(new ECBMapPlayer(ligne[0]));
			else if(key == "MAP") recv_map = true;
			else if(key == "BEGIN") begin_money = atoi(ligne.c_str());
			else if(key == "CITY") city_money = atoi(ligne.c_str());
			else if(key == "MIN") min = atoi(ligne.c_str());
			else if(key == "MAX") max = atoi(ligne.c_str());
			else
				throw ECExcept(VName(key) VName(name), "Fichier map incorrect");
		}
		else
		{
			if(!x || !y)
				throw ECExcept(VIName(x) VIName(y) VName(name), "Fichier map incorrect");
			if(map_players.empty())
				throw ECExcept(VIName(map_players.size()), "Aucun player !?");
			if(_y >= y)
				throw ECExcept(VIName(_y) VIName(y), "Trop de cases");
			uint size = ligne.size();
			_x = 0;
			uint counter = 0;
			ECBCase *acase = 0;
			ECBCountry *country = 0;
			char c_id[3];
			for(uint i=0; i<size; ++i)
			{
				if(_x >= x)
						throw ECExcept(VIName(_x) VIName(x), "Trop de cases");

				/* WXXYZ
				 * W(terrain), XX(country), Y(owner), Z(architecture)
				 */
				switch(counter)
				{
					/* Type du terrain (utilisation de case_type */
					case 0:
					{
						for(uint j=0; j < (sizeof case_type / sizeof *case_type); j++)
							if(case_type[j].c == ligne[i])
							{
								acase = case_type[j].func (this, _x, _y, case_type[j].flags, case_type[j].c);
								break;
							}
						if(!acase)
							throw ECExcept(VPName(acase) VCName(ligne[i]), "Terrain introuvable");
						map.push_back(acase);
						break;
					}
					/* R�cup�ration de l'identification de la Country (premi�re partie) */
					case 1:
					{
						c_id[0] = ligne[i];
						c_id[1] = '\0', c_id[2] = '\0';
						break;
					}
					/* Identification de la Country */
					case 2:
					{
						c_id[1] = ligne[i];
						std::vector<ECBCountry*>::iterator it;
						for(it = map_countries.begin();
						    it != map_countries.end() && strcmp(c_id, (*it)->ID()); ++it);
						if(it == map_countries.end())
						{ /* La country n'existe pas encore */
							country = new ECBCountry(c_id);
							map_countries.push_back(country);
						}
						else /* La country existe d�j� */
							country = *it;
						acase->SetCountry(country);
						country->AddCase(acase);
						break;
					}
					/* Player owner (utilisation de map_players) */
					case 3:
					{
						std::vector<ECBMapPlayer*>::iterator it;
						for(it=map_players.begin();
						    it != map_players.end() && (*it)->ID() != ligne[i]; it++);
						if(it == map_players.end())
							throw ECExcept(VCName(ligne[i]) VIName(map_players.size()), "Player introuvable !?");

						if(!country->Owner())
							country->SetOwner(*it);
						else if(country->Owner() != *it)
							throw ECExcept(VCName(country->Owner()->ID()) VCName((*it)->ID())
							               VIName(_x) VIName(_y), "Une country a deux owners !");

						if((*it)->FindCountry(c_id)) /* Ce MapPlayer a d�j� cette country en m�moire */
							break;
						(*it)->AddCountry(country);
						break;
					}
					/* Type d'architecture */
					case 4:
					{
						/* C'est une fonction virtuelle qui dans la lib ne fait rien.
						 * Elle sera surpass�e dans le client pour d�finir les attributs propres
						 * aux images.
						 */
						SetCaseAttr(acase, ligne[i]);
						break;
					}
					default:
						throw ECExcept(VIName(counter), "Compteur anormal !?");
				}

				if(counter >= 4)
				{
					_x++;
					acase = 0;
					counter = 0;
				}
				else counter++;
			}
			if(_x < x)
				throw ECExcept(VIName(_y) VIName(_x) VIName(x), "Il n'y a pas assez de case sur cette ligne");
			_y++;
		}
	}
	/* V�rification finale des donn�es */
	if(!begin_money || !city_money || !x || !y || map.empty() || map_players.empty() || map_countries.empty() ||
	   !min || !max || map_players.size() != max || min > max)
		throw ECExcept(VIName(begin_money) VIName(city_money) VIName(x) VIName(y) VIName(map.size()) VIName(min) VIName(max)
		               VIName(map_players.size()) VIName(map_countries.size()), "Fichier incorrect !");

	/* On v�rifie si il y a bien une ville par country */
	for(std::vector<ECBCountry*>::iterator it= map_countries.begin(); it != map_countries.end(); ++it)
	{
		uint count = 0;
		std::vector<ECBCase*> acase = (*it)->Cases();
		std::vector<ECBCase*>::iterator cc;
		for(cc = acase.begin(); cc != acase.end(); ++cc)
			if((*cc)->Flags() & C_VILLE)
				count++;
		if(count != 1)
			throw ECExcept(VIName(count) VSName((*it)->ID()),
			               count > 1 ? "La country a trop de villes !" : "La country n'a pas de ville !");
	}

	/* La map est *bien* initialis�e !! */
	initialised = true;
}

ECBMap::~ECBMap()
{
	/* Lib�ration des cases */
	for(std::vector<ECBCase*>::iterator it=map.begin(); it != map.end(); ++it)
		delete *it;

	/* Lib�ration des MapPlayers */
	for(std::vector<ECBMapPlayer*>::iterator it=map_players.begin(); it != map_players.end(); ++it)
		delete *it;
		
	/* Lib�ration des Countries */
	for(std::vector<ECBCountry*>::iterator it=map_countries.begin(); it != map_countries.end(); ++it)
		delete *it;
}

ECBCase* ECBMap::operator() (uint _x, uint _y) const
{
	if(!initialised)
		throw ECExcept(0, "ECBMap n'est pas initialis�");

	if (_x >= x || _y >= y)
		throw ECExcept(VIName(x) VIName(y) VIName(_x) VIName(_y), "Access � un element hors du tableau");

	/** \warning ALIGNEMENT FAIT LIGNE PAR LIGNE !!! */
	return map[ _y * x + _x ];
}
