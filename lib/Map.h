/* lib/Map.h - Header of Map.cpp
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

#ifndef ECLIB_MAP_H
#define ECLIB_MAP_H

#include "Channels.h"
#include <string>
#include <vector>

/** @page Map_structure Map structure
 *
 * All data are organized like this :
 * <pre>
 * ECBMap
 *  |- std::vector<ECBCases*>  <-----------
 *  |- std::vector<ECBCountry*>  <----     \
 *  `- std::vector<ECBMapPlayer*>     \    |
 *      `- std::vector<ECBCountry*> ---    /
 *          `- std::vector<ECBCases*> -----
 * </pre>
 */

class ECBMap;

class ECBMapPlayer;

class ECBCountry;

/********************************************************************************************
 *                               ECBCase                                                    *
 ********************************************************************************************/

#define C_VILLE          0x0001
#define C_CAPITALE       0x0002
#define C_TERRE          0x0004
#define C_MER            0x0008
#define C_PONT           0x0010

/** This is a Case's class...
 */
class ECBCase
{
/* Constructeur/Destructeur */
public:

	ECBCase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id);

/* Methodes */
public:

/* Attributs */
public:

	/** This case is a member of this map */
	ECBMap* Map() { return map; }

	char TypeID() { return type_id; }

	uint Flags() { return flags; }

	ECBCountry* Country() { return map_country; }
	void SetCountry(ECBCountry *mc) { map_country = mc; }

/* Variables privées */
protected:

	ECBMap *map;

	uint x, y;

	uint flags;

	char type_id;

	ECBCountry *map_country;
};

/** This class is a derived class from ECBCase whose is a city */
class ECBVille : public ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBVille(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is a land */
class ECBTerre : public ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBTerre(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is sea */
class ECBMer : public ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBMer(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is a bridge */
class ECBPont : public ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBPont(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/********************************************************************************************
 *                               ECBCountry                                                 *
 ********************************************************************************************/
/** This class is a country in the map
 *
 * \warning \a cases is a vector of pointers to ECBMap::map, so we don't have to delete them.
 */
class ECBCountry
{
/* Constructeur/Destructeur */
public:

	ECBCountry(char _ident[3]);

/* Attributs */
public:

	void AddCase(ECBCase* _case) { cases.push_back(_case); }
	
	const char* ID() { return ident; }

	ECBMapPlayer* Owner() { return owner; }
	void SetOwner(ECBMapPlayer* mp) { owner = mp; }

	std::vector<ECBCase*> Cases() { return cases; }

/* Variables privées */
protected:
	std::vector<ECBCase*> cases;
	ECBMapPlayer *owner;
	char ident[3];
};

/********************************************************************************************
 *                               ECBMapPlayer                                               *
 ********************************************************************************************/
/** This is a Map Player to identify him on the map
 *
 * \warning \a countries is a vector of pointers to ECBMap::map_countries, so we don't have to delete them.
 */
class ECBMapPlayer
{
/* Constructeur/Destructeur*/
public:

	ECBMapPlayer(char _id)
		: id(_id)
	{}

/* Attributs */
public:

	char ID() { return id; }

	void AddCountry(ECBCountry* _country) { countries.push_back(_country); }

	ECBCountry* FindCountry(const char*);

	std::vector<ECBCountry*> Countries() { return countries; }

/* Variables privées */
public:
	char id;
	std::vector<ECBCountry*> countries;

};

/********************************************************************************************
 *                               ECBMap                                                     *
 ********************************************************************************************/

/** This is a base of Map class.
 *
 * \warning there is a lot of exceptions in constructor ! Si you *must* use a try/except block when you
 *          create an ECBMap !
 */
class ECBMap
{
/* Constructeur/Destructeur */
public:

	/** Path of map (used only by Server) */
	ECBMap(std::string _filename);

	/** Constructor from a string's vector
	 * @param _map_file this is a string's vector where there is informations about map
	 */
	ECBMap(std::vector<std::string> _map_file);

	~ECBMap();

/* Methodes */
public:

/* Attributs */
public:

	/** Name of channel */
	std::string Name() { return name; }

	/** Channel linked to map */
	ECBChannel* Channel() { return chan; }
	
	uint BeginMoney() { return begin_money; }          /**< All players have this money when they begin the game */
	uint CityMoney() { return city_money; }            /**< This is money that is given to players at each turn by cities */
	uint MinPlayers() { return min; }                  /**< Min players to play */
	uint MaxPlayers() { return max; }                  /**< Max players to play */
	uint NbCases() { return map.size(); }              /**< Number of cases */
	uint NbMapPlayers() { return map_players.size(); } /**< Number of map players */
	uint NbCountries() { return map_countries.size(); }/**< Number of map countries */

	uint Width() { return x; }
	uint Height() { return y; }

	std::vector<ECBCase*> Cases() { return map; }                   /**< Return case vector */
	std::vector<ECBMapPlayer*> MapPlayers() { return map_players; } /**< Return map players vector */
	std::vector<ECBCountry*> Countries() { return map_countries; }  /**< Return countries vector */
	
	/** Access to a case of map 
	 * Example: map(x,y)
	 */
	ECBCase* operator() (uint x, uint y) const;

/* Variables privées */
protected:
	std::vector<ECBCase*> map;              /**< Alignement fait ligne par ligne !!! */

	std::vector<std::string> map_file;
	
	std::vector<ECBMapPlayer*> map_players;
	
	std::vector<ECBCountry*> map_countries;

	ECBChannel *chan;
	std::string name;

	uint x, y;
	uint min, max;

	uint begin_money, city_money;

	bool initialised; /**< This variable is setted to true only when \a map is empty */

	void Init();      /**< Initialisation, called by constructors */

	ECBMap();         /**< Disallowed */
};

#endif /* ECLIB_MAP_H */
