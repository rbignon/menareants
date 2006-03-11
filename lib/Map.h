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

#include <string>
#include <vector>
#include "Outils.h"

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

class ECBPlayer;
class ECBChannel;
class ECBMap;
class ECBMapPlayer;
class ECBCountry;
class ECBCase;
class ECBEntity;

typedef std::vector<ECBCase*> BCaseVector;
typedef std::vector<ECBMapPlayer*> BMapPlayersVector;
typedef std::vector<ECBCountry*> BCountriesVector;
typedef std::vector<ECBEntity*> BEntityVector;
typedef char Entity_ID[3];
typedef char Country_ID[3];
typedef char MapPlayer_ID;

/********************************************************************************************
 *                               ECBDate                                                    *
 ********************************************************************************************/

class ECBDate
{
/* Constructeur/Destructeur */
public:
	ECBDate(uint d, uint m, int y);
	ECBDate(std::string date);

/* Methodes */
public:

	std::string String();

	void CheckDate();

/* Attributs */
public:

	uint Day() { return d; }
	uint Month() { return m; }
	uint Year() { return y; }

	void SetDay(uint _d) { d = _d; }
	void SetMonth(uint _m) { m = _m; }
	void SetYear(int _y) { y = _y; }

	void SetDate(std::string date);

	ECBDate& operator++ ();    // prefix ++
	ECBDate  operator++ (int); // postfix ++

/* Variables privées */
protected:
	uint d;
	uint m;
	int y;
};

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

	/** Return type of terrain (a char) */
	char TypeID() { return type_id; }

	/** Return flags of case */
	uint Flags() { return flags; }

	ECBCountry* Country() { return map_country; }
	void SetCountry(ECBCountry *mc) { map_country = mc; }

	ECList<ECBEntity*> *Entities() { return &entities; }

	uint X() { return x; }
	uint Y() { return y; }

	ECBCase* MoveUp(uint c = 1);
	ECBCase* MoveDown(uint c = 1);
	ECBCase* MoveLeft(uint c = 1);
	ECBCase* MoveRight(uint c = 1);

/* Variables privées */
protected:

	ECBMap *map;

	uint x, y;

	uint flags;

	char type_id;

	ECBCountry *map_country;

	ECList<ECBEntity*> entities;
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
 *                               ECBEntity                                                  *
 ********************************************************************************************/
class ECBEntity
{
/* Constructor/Destructor */
public:

	enum e_type {
		E_ARMY,
		E_END
	};

	ECBEntity() {}

	ECBEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type type, uint Step, uint nb = 0);

	virtual ~ECBEntity() {}

/* Methodes */
public:

	virtual ECBCase* Attaq(uint x, uint y) = 0;

	virtual ECBCase* Move(uint x, uint y) = 0;

	virtual bool Return() = 0;

	virtual bool CanAttaq(ECBEntity* e) = 0;

	virtual void CreateLast() = 0;

	/** This function remove entity from case where it is and add it
	 *  it the new case.
	 * @param new_case this is the new case where we send our entity
	 * @param last_case this is the last case where our entity is from. If there
	 *                  is nothing here, it will take "acase"
	 */
	void ChangeCase(ECBCase* new_case, ECBCase* last_case = 0);

/* Attributs */
public:

	void SetCase(ECBCase* _c) { acase = _c; }
	ECBCase* Case() { return acase; }

	void SetOwner(ECBPlayer* _p) { owner = _p; }
	ECBPlayer* Owner() { return owner; }

	const char* ID() { return name; }

	e_type Type() { return type; }

	/** Return the number of soldats in the army */
	uint Nb() { return nb; }
	void SetNb(uint n) { nb = n; }

	/** This unit is locked, because it is new, or deleted, or in a move, or in a transport. */
	bool Locked() { return lock; }
	void SetLock(bool l = true) { lock = l; }

	/** Return last entity */
	ECBEntity* Last() { return last; }
	void RemoveLast() { MyFree(last); }

	uint MyStep() { return myStep; }
	void SetMyStep(uint s) { myStep = s; }
	uint RestStep() { return restStep; }
	void SetRestStep(uint s) { restStep = s; }
	

/* Variables protégées */
protected:
	ECBPlayer* owner;
	Entity_ID name;
	ECBCase *acase;
	e_type type;
	ECBEntity* last;
	uint nb;
	bool lock;
	uint myStep;
	uint restStep;

	bool SetLast(ECBEntity* e) { return (!last) ? (last = e) : false; }
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

	ECBCountry(const Country_ID ident);

/* Attributs */
public:

	/** Add case to country */
	void AddCase(ECBCase* _case) { cases.push_back(_case); }

	/** Return country ID (two chars) */
	const char* ID() { return ident; }

	/** Return Owner of country */
	ECBMapPlayer* Owner() { return owner; }
	void SetOwner(ECBMapPlayer* mp) { owner = mp; }

	std::vector<ECBCase*> Cases() { return cases; }

/* Variables privées */
protected:
	std::vector<ECBCase*> cases;
	ECBMapPlayer *owner;
	Country_ID ident;
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

	ECBMapPlayer(char _id, uint _num)
		: id(_id), num(_num), pl(0)
	{}

/* Attributs */
public:

	MapPlayer_ID ID() { return id; }

	uint Num() { return num; }

	void AddCountry(ECBCountry* _country) { countries.push_back(_country); }

	/** Find a country by ID */
	ECBCountry* FindCountry(const char*id);

	ECBPlayer* Player() { return pl; }
	void SetPlayer(ECBPlayer* _pl) { pl = _pl; }

	std::vector<ECBCountry*> Countries() { return countries; }

/* Variables privées */
public:
	MapPlayer_ID id;
	uint num;
	ECBPlayer* pl;
	std::vector<ECBCountry*> countries;

};

/********************************************************************************************
 *                               ECBMap                                                     *
 ********************************************************************************************/

/** This is a base of Map class.
 *
 * \warning there are a lot of exceptions in constructor ! You *must* use a try/except block when you
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

	virtual ~ECBMap();

/* Methodes */
public:

	/** Delete all MapPlayers who are not used by a real player */
	void ClearMapPlayers();

/* Attributs */
public:

	/** Name of channel */
	std::string Name() { return name; }

	/** Channel linked to map */
	ECBChannel* Channel() { return chan; }

	int BeginMoney() { return begin_money; }           /**< All players have this money when they begin the game */
	int CityMoney() { return city_money; }             ///< This is money that is given to players at each turn by cities
	uint MinPlayers() { return min; }                  /**< Min players to play */
	uint MaxPlayers() { return max; }                  /**< Max players to play */
	uint NbCases() { return map.size(); }              /**< Number of cases */
	uint NbMapPlayers() { return map_players.size(); } /**< Number of map players */
	uint NbCountries() { return map_countries.size(); }/**< Number of map countries */

	uint NbSoldats() { return nb_soldats; }            /**< This is the number of initiales soldats in armies */

	uint Width() { return x; }
	uint Height() { return y; }

	BCaseVector Cases() { return map; }                             /**< Return case vector */
	BMapPlayersVector MapPlayers() { return map_players; }          /**< Return map players vector */
	BCountriesVector Countries() { return map_countries; }          /**< Return countries vector */

	ECList<ECBEntity*> *Entities() { return &entities; }

	std::vector<std::string> MapFile() { return map_file; }         /**< Return map_file vector */

	ECBDate* Date() { return date; }
	ECBDate* NextDay() { return &(++(*date)); }                     /**< Increment date to next day */

	std::vector<std::string> MapInfos() { return map_infos; }       /**< Map informations */
	
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

	std::vector<std::string> map_infos;

	ECList<ECBEntity*> entities;

	ECBDate* date;

	ECBChannel *chan;
	std::string name;

	uint x, y;
	uint min, max;
	uint nb_soldats;

	uint begin_money, city_money;

	bool initialised; /**< This variable is setted to true only when \a map is empty */

	void Init();      /**< Initialisation, called by constructors */

	/** This function have to be redefined by client to set attributs of images in case */
	virtual void SetCaseAttr(ECBCase*, char) {}

	ECBMap();         /**< Disallowed */
};

#endif /* ECLIB_MAP_H */
