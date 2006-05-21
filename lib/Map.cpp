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
#include "Channels.h"
#include <fstream>

/********************************************************************************************
 *                                 ECBMove                                                  *
 ********************************************************************************************/

ECBMove::ECBMove(ECBEntity* e)
	: first_case(0), entity(e)
{
	if(e) first_case = e->Case();
}

std::string ECBMove::MovesString(ECBCase* end)
{
	ECBCase* c = first_case;
	if(!c) return "";
	std::string s;
	for(Vector::const_iterator it = moves.begin(); it != moves.end() && (!end || end != c); ++it)
		switch(*it)
		{
			case Up: s += '^'; c = c->MoveUp(); break;
			case Down: s += 'v'; c = c->MoveDown(); break;
			case Left: s += '<'; c = c->MoveLeft(); break;
			case Right: s += '>'; c = c->MoveRight(); break;
		}
	return s;
}

void ECBMove::Return(ECBCase* end)
{
	if(end == first_case)
	{
		Clear();
		return;
	}

	ECBCase* c = first_case;
	if(!c) return;
	bool found = false;

	for(Vector::iterator it = moves.begin(); it != moves.end();)
	{
		if(!found)
		{
			switch(*it)
			{
				case Up: c = c->MoveUp(); break;
				case Down: c = c->MoveDown(); break;
				case Left: c = c->MoveLeft(); break;
				case Right: c = c->MoveRight(); break;
			}
			if(c == end) found = true;
			++it;
		}
		else
			it = moves.erase(it);
	}
	if(moves.empty())
		first_case = 0;
}

/********************************************************************************************
 *                               ECBDate                                                    *
 ********************************************************************************************/

ECBDate::ECBDate(uint _d, uint _m, int _y)
	: d(_d), m(_m), y(_y)
{
	CheckDate();
}

ECBDate::ECBDate(std::string date)
{
	d = 0;
	m = 0;
	y = 0;
	SetDate(date);
}

ECBDate& ECBDate::operator++ ()
{
	bool incm = false;
	switch(m)
	{
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			if(d == 31) incm = true;
			break;
		case 2:
			if(d == 28) incm = true;
			break;
		default:
			if(d == 30) incm = true;
	}
	if(incm)
	{
		d = 1;
		if(m == 12)
		{
			m = 1;
			y++;
		}
		else m++;
	}
	else
		d++;
	return *this;
}

ECBDate ECBDate::operator++ (int)
{
	ECBDate ans = *this;
	++(*this);  // on appelle simplement operator++()
	return ans;
}

std::string ECBDate::String()
{
	return (TypToStr(d) + "/" + TypToStr(m) + "/" + TypToStr(y));
}

void ECBDate::CheckDate()
{
	if(!d || !m || d > 31 || d > 28 && m == 2 || m > 12)
		throw ECExcept(VIName(d) VIName(m), "Date incorrecte");

	switch(m)
	{
		case 4: case 5: case 9: case 11:
			if(d > 30)
				throw ECExcept(VIName(d) VIName(m), "Date incorrecte");
			break;
	}
}

void ECBDate::SetDate(std::string date)
{
	unsigned short n = 0;
	while(n<3)
	{
		std::string s = stringtok(date, " /");
		if(s.empty() || !is_num(s.c_str()))
			throw ECExcept(VName(s), "Date incorrecte");
		switch(n)
		{
			case 0: d = StrToTyp<uint>(s); break;
			case 1: m = StrToTyp<uint>(s); break;
			case 2: y = StrToTyp<int>(s); break;
		}
		n++;
	}
	CheckDate();
}

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

ECBCase* ECBCase::MoveUp(uint c)    { return y >= c ? (*map)(x, y-c) : this; }
ECBCase* ECBCase::MoveDown(uint c)  { return y < map->Height()-c-1 ? (*map)(x, y+c) : this; }
ECBCase* ECBCase::MoveLeft(uint c)  { return x >= c ? (*map)(x-c, y) : this; }
ECBCase* ECBCase::MoveRight(uint c) { return x < map->Width()-c-1 ? (*map)(x+c, y) : this; }

void ECBCase::SetCountry(ECBCountry *mc)
{
	map_country = mc;
}

void ECBCase::CheckChangingOwner(ECBEntity* e)
{
	std::vector<ECBEntity*> ents = entities.List();
	for(std::vector<ECBEntity*>::const_iterator enti = ents.begin(); enti != ents.end(); ++enti)
		if((*enti)->IsCountryMaker())
		{
			if(e->Owner() && (!Country()->Owner() ||
			    (Country()->Owner()->Player() != e->Owner() && !e->Owner()->IsAllie(Country()->Owner()->Player()))))
				Country()->ChangeOwner(e->Owner()->MapPlayer());
			break;
		}
}

/********************************************************************************************
 *                               ECBEntity                                                  *
 ********************************************************************************************/

ECBEntity::ECBEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint Step, uint _c, uint _nb)
	: owner(_owner), acase(_case), type(_type), nb(_nb), lock(false), deployed(false), shooted(0), cost(_c), event_type(0)
{
	if(strlen(_name) != (sizeof name)-1)
		throw ECExcept(VIName(strlen(_name)) VSName(_name), "ID trop grand ou inexistant.");
	strcpy(name, _name);
	myStep = Step;
	restStep = Step;
}

void ECBEntity::SetID(const char* _name)
{
	if(strlen(_name) != (sizeof name)-1)
		throw ECExcept(VIName(strlen(_name)) VSName(_name), "ID trop grand ou inexistant.");
	strcpy(name, _name);
}

bool ECBEntity::Like(ECBEntity* e)
{
	return (owner == e->Owner() || owner && owner->IsAllie(e->Owner()));
}

void ECBEntity::Played()
{
	restStep = myStep;
	event_type = 0;
}

bool ECBEntity::AddUnits(uint u)
{
	nb += u;
	return true;
}

bool ECBEntity::CanBeCreated(ECBPlayer* pl) const
{
	/** \todo avec les nations, vérifier ici si ma nation peut créer cette unité, car si
	 * ce n'est pas le cas autant se barrer de suite.
	 */
	return true;
}

bool ECBEntity::CanBeCreated(ECBCase* c) const
{
	if(!c)
		c = acase;
	assert(c);

	if(owner && !CanBeCreated(owner))
		return false;

	bool ret = false;
	std::vector<ECBEntity*> entv = c->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
	{
		if((*enti)->CanCreate(this) && (*enti)->Owner() == owner)
			ret = true;
		/* On vérifie que :
		 * - si les deux sont des batiments et que ce ne sont pas les memes
		 * - si les deux ne sont pas des batiments et que ce ne sont pas les memes
		 * on ne peut pas construire, car deux unités du meme "type" ne peuvent pas
		 * cohabiter sur la meme case
		 */
		if((*enti)->IsBuilding() && this->IsBuilding() && *enti != this)
			return false;
	}

	/* Si la case sur laquelle je suis est au meme joueur que l'entité et que c'est
	 * une case qui permet de créer une unité de ce type (donc ville etc)
	 */
	if(!ret && c->Country()->Owner() && c->Country()->Owner()->Player() == owner && c->CanCreate(this))
		return true;

	return ret;
}

void ECBEntity::ChangeCase(ECBCase* new_case)
{
	assert(new_case && acase);

	acase->Entities()->Remove(this);
	new_case->Entities()->Add(this);
	acase = new_case;
}

std::string ECBEntity::LongName()
{
	return std::string(Owner() ? Owner()->GetNick() : "*") + "!" + ID();
}

/********************************************************************************************
 *                               ECBCountry                                                 *
 ********************************************************************************************/

ECBCountry::ECBCountry(ECBMap* _map, const Country_ID _ident)
	: map(_map)
{
	strcpy(ident, _ident);
	owner = 0;
}

bool ECBCountry::ChangeOwner(ECBMapPlayer* mp)
{
	if(Owner())
		Owner()->RemoveCountry(this);

	SetOwner(mp);
	if(Owner())
		mp->AddCountry(this);
	return true;
}

void ECBCountry::RemoveCase(ECBCase* _case)
{
	for (std::vector<ECBCase*>::iterator it = cases.begin(); it != cases.end(); )
	{
		if (*it == _case)
		{
			it = cases.erase(it);
			return;
		}
		else
			++it;
	}
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

bool ECBMapPlayer::RemoveCountry(ECBCountry* _country, bool use_delete)
{
	for (std::vector<ECBCountry*>::iterator it = countries.begin(); it != countries.end(); )
	{
		if (*it == _country)
		{
			if(use_delete)
				delete _country;
			it = countries.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
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
	{ 'm', CreateCase<ECBMer>,   C_MER              },
	{ 't', CreateCase<ECBTerre>, C_TERRE            },
	{ 'p', CreateCase<ECBPont>,  C_PONT             }
};

ECBCase* ECBMap::CreateCase(uint _x, uint _y, char type_id)
{
	for(uint j=0; j < (sizeof case_type / sizeof *case_type); j++)
		if(case_type[j].c == type_id)
			return case_type[j].func (this, _x, _y, case_type[j].flags, case_type[j].c);

	return 0;
}

ECBMap::ECBMap(std::vector<std::string> _map_file)
{
	initialised = false;

	map_file = _map_file;
}

ECBMap::ECBMap(std::string _filename)
	: filename(_filename)
{
	initialised = false;

	std::ifstream fp(filename.c_str());

	if(!fp)
		throw ECExcept(VName(filename), "Impossible d'ouvrir le fichier");

	std::string ligne;

	while(std::getline(fp, ligne))
		if(ligne[0] != '#' && ligne[0] != '\0')
			map_file.push_back(ligne);
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
 *   DATE [jour] [mois] [année]
 *   INFO [ligne]
 *   INFO [ligne]
 *   ..
 *   INFO [ligne]
 *   INFO [ligne]
 *   UNIT [type] [owner] [x],[y] [number]
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
 *      'm' = mer.
 *      't' = terre.
 *      'p' = pont.
 *   XX = Identificator of country (like AA, AB, etc)
 *   Y = Player Owner (like A, B, C, etc, or * for 'neutral')
 *   Z = Type of architecture (only for image, so not used by server)
 *   </pre>
 *
 *  6) BEGIN [begining money]
 *   <pre>
 *   Here we define money that all players have when they begin game.
 *   </pre>
 *
 *  7) CITY [money by city]
 *   <pre>
 *   Here we put money win by each city.
 *   </pre>
 *
 *  8) MIN [min]
 *   <pre>
 *   Min players.
 *   </pre>
 *
 *  6) MAX [max]
 *   <pre>
 *   Max players. It's channel limit.
 *   </pre>
 *
 *  7) DATE [jour] [mois] [année]
 *   <pre>
 *   Initial date of game.
 *   </pre>
 *
 *  8) INFO [ligne]
 *   <pre>
 *   This is one of lines who is a short text of map.
 *   </pre>
 *
 *  9) UNIT [type] [owner] [x],[y] [number]
 *   <pre>
 *   Put an unit on the map, on a case, to one player.
 *   </pre>
 *
 * @author Progs
 * @date 15/02/2006
 */
void ECBMap::Init()
{
	if(initialised)
		throw ECExcept(VBName(initialised), "Appel de la fonction alors que la carte est déjà initialisée");

	chan = 0;
	x = 0;
	y = 0;
	min = 0;
	max = 0;
	date = 0;
	begin_money = 0;
	city_money = 0;
	
	std::string ligne;

	bool recv_map = false;
	uint _x = 0, _y = 0, num_player = 0;

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
			{
				if(!isalpha(ligne[0]))
					throw ECExcept(VCName(ligne[0]), "L'identifiant de ce player n'est pas correct "
					                                 "(doit être une lettre)");
				BMapPlayersVector::iterator it = map_players.begin();
				for(; it != map_players.end() && (*it)->ID() != ligne[0]; ++it);
				if(it != map_players.end())
					throw ECExcept(VCName(ligne[0]), "L'identifiant est déjà utilisé");
				map_players.push_back(CreateMapPlayer(ligne[0], ++num_player));
			}
			else if(key == "MAP") recv_map = true;
			else if(key == "BEGIN") begin_money = atoi(ligne.c_str());
			else if(key == "CITY") city_money = atoi(ligne.c_str());
			else if(key == "MIN") min = atoi(ligne.c_str());
			else if(key == "MAX") max = atoi(ligne.c_str());
			else if(key == "DATE") date = new ECBDate(ligne);
			else if(key == "INFO") map_infos.push_back(ligne);
			else if(key == "UNIT")
			{
				std::string line = ligne;
				std::string type = stringtok(line, " ");
				std::string owner = stringtok(line, " ");
				if(owner[0] == '*')
					AddNeutralUnit(ligne);
				else
				{
					BMapPlayersVector::iterator it = map_players.begin();
					for(; it != map_players.end() && (*it)->ID() != owner[0]; ++it);
					if(it == map_players.end())
						throw ECExcept(VName(owner), "Déclaration d'une unité pour un owner qui n'existe pas");
	
					(*it)->AddUnit(ligne);
				}
				VirtualAddUnit(ligne);
			}
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
			Country_ID c_id; // [3]
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
						if(!(acase = CreateCase(_x, _y, ligne[i])))
							throw ECExcept(VPName(acase) VCName(ligne[i]), "Terrain introuvable");
						map.push_back(acase);
						break;
					}
					/* Récupération de l'identification de la Country (première partie) */
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
							country = CreateCountry(this, c_id);
							map_countries.push_back(country);
						}
						else /* La country existe déjà */
							country = *it;
						acase->SetCountry(country);
						country->AddCase(acase);
						break;
					}
					/* Player owner (utilisation de map_players) */
					case 3:
					{
						if(ligne[i] == '*')
						{ /* C'est une country *neutre* */
							if(country->Owner())
								throw ECExcept(VCName(ligne[i]) VIName(_x) VIName(_y) VSName(country->ID()),
								               "Cette country est un coup neutre un coup appartient à quelqu'un ?");
							else
								break;
						}

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

						if(!(*it)->FindCountry(c_id)) /* Ce MapPlayer n'a pas encore cette country en mémoire */
							(*it)->AddCountry(country);
						break;
					}
					/* Type d'architecture */
					case 4:
					{
						/* C'est une fonction virtuelle qui dans la lib ne fait rien.
						 * Elle sera surpassée dans le client pour définir les attributs propres
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
	/* Vérification finale des données */
	if(!begin_money || !city_money || !x || !y || map.empty() || map_players.empty() || map_countries.empty() ||
	   !min || !max || map_players.size() != max || min > max || !date)
		throw ECExcept(VIName(map_players.size()) VIName(city_money) VIName(x) VIName(y) VIName(map.size()) VIName(min)
		               VIName(max) VIName(begin_money) VIName(map_countries.size()) VPName(date),
		               "Fichier incorrect !");

#if 0 /** \todo les villes sont des unités, il faut voir si on ne fait pas une vérification auprès des UNIT */
	/* On vérifie si il y a bien une ville par country */
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
#endif

	/* La map est *bien* initialisée !! */
	initialised = true;
}

void ECBMap::ClearMapPlayers()
{
	for(BMapPlayersVector::iterator it=map_players.begin(); it != map_players.end();)
		if(!(*it)->Player())
		{
			BCountriesVector countries = (*it)->Countries();
			for(BCountriesVector::iterator cvi = countries.begin(); cvi != countries.end(); ++cvi)
				(*cvi)->SetOwner(NULL);

			std::vector<std::string> sts = (*it)->Units();
			for(std::vector<std::string>::iterator st = sts.begin(); st != sts.end(); ++st)
				neutres_units.push_back(*st);

			delete *it;
			it = map_players.erase(it);
		}
		else ++it;
}

void ECBMap::Reload()
{
	/* On libère tout ce qui est ouvert pour tout recharger à partir des lignes */
	Destruct();
	Init();
}

void ECBMap::Destruct()
{
	/* Libération des entitées */
	entities.Clear(USE_DELETE);

	/* Libération des cases */
	for(std::vector<ECBCase*>::iterator it=map.begin(); it != map.end(); ++it)
		delete *it;
	map.clear();

	/* Libération des MapPlayers */
	for(std::vector<ECBMapPlayer*>::iterator it=map_players.begin(); it != map_players.end(); ++it)
		delete *it;
	map_players.clear();
		
	/* Libération des Countries */
	for(std::vector<ECBCountry*>::iterator it=map_countries.begin(); it != map_countries.end(); ++it)
		delete *it;
	map_countries.clear();

	if(initialised)
		delete date;
	initialised = false;
}

ECBMap::~ECBMap()
{
	Destruct();
}

ECBCase*& ECBMap::operator() (uint _x, uint _y)
{
	if(!initialised)
		throw ECExcept("", "ECBMap n'est pas initialisé");

	if (_x >= x || _y >= y)
		throw ECExcept(VIName(x) VIName(y) VIName(_x) VIName(_y), "Access à un element hors du tableau");

	/** \warning ALIGNEMENT FAIT LIGNE PAR LIGNE !!! */
	return map[ _y * x + _x ];
}

void ECBMap::AddAnEntity(ECBEntity* e)
{
	if(!e) return;

	e->Case()->Entities()->Add(e);
	if(e->Owner())
		e->Owner()->Entities()->Add(e);
	else
		neutres.Add(e);
	entities.Add(e);
}

void ECBMap::RemoveAnEntity(ECBEntity* e, bool use_delete)
{
	if(!e) return;

	e->Case()->Entities()->Remove(e);
	entities.Remove(e);
	if(e->Owner())
		e->Owner()->Entities()->Remove(e);
	else
		neutres.Remove(e);
	if(use_delete)
		delete e;
}

