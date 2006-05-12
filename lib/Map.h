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
 *                                   ECBMove                                                *
 ********************************************************************************************/

class ECBMove
{
/* Constructeur/Destructeur */
public:

	ECBMove(ECBEntity* e = 0);
	virtual ~ECBMove() {}

	enum E_Move {
		Up,
		Down,
		Left,
		Right
	};
	typedef std::vector<E_Move> Vector;

/* Methodes */
public:

/* Attributs */
public:

	bool Empty() const { return moves.empty(); }
	Vector::size_type Size() const { return moves.size(); }
	Vector Moves() const { return moves; }
	void AddMove(E_Move m) { moves.push_back(m); }
	void SetMoves(Vector _moves) { moves = _moves; }
	void SetEntity(ECBEntity* et) { entity = et; }
	ECBEntity* Entity() { return entity; }

	ECBCase* FirstCase() { return first_case; }
	void SetFirstCase(ECBCase* c) { first_case = c; }

	std::string MovesString(ECBCase* end = 0);

	virtual void Clear(ECBCase* c = 0) { moves.clear(); first_case = c; }
	void Return(ECBCase*);

/* Variables privées */
protected:
	Vector moves;
	ECBCase* first_case;
	ECBEntity* entity;
};

/********************************************************************************************
 *                               ECBEntity                                                  *
 ********************************************************************************************/
#define ThereIsAttaq(a, b) ((a)->CanAttaq(b) && !(a)->Like(b) || \
                            (b)->CanAttaq(a) && !(b)->Like(a))
#define ARM_MOVE        0x0001
#define ARM_SPLIT       0x0002
#define ARM_ATTAQ       0x0004
#define ARM_REMOVE      0x0008
#define ARM_LOCK        0x0010
#define ARM_TYPE        0x0020
#define ARM_NUMBER      0x0040
#define ARM_RETURN      0x0080
#define ARM_DEPLOY      0x0800
#define ARM_PREUNION    (ARM_MOVE|ARM_LOCK)
#define ARM_UNION       (ARM_MOVE|ARM_REMOVE)
#define ARM_CREATE      (ARM_MOVE|ARM_TYPE|ARM_NUMBER)
/* Concerne server:EChannel::SendArm() */
#define ARM_HIDE        0x0100 /* cache les infos aux users non concernés */
#define ARM_RECURSE     0x0200 /* ne JAMAIS appeler */
#define ARM_NOCONCERNED 0x0400 /* ne pas envoyer aux users concernés */
class ECBEntity
{
/* Constructor/Destructor */
public:

	enum e_type {
		E_NONE,
		E_ARMY,
		E_CASERNE,
		E_CHARFACT,
		E_CHAR,
		E_MISSILAUNCHER,
		E_END
	};

	ECBEntity(e_type t = E_NONE, uint _cost = 0)
		: owner(0), acase(0), type(t), nb(0), lock(false), deployed(false), shooted(0), cost(_cost), event_type(0)
	{}

	ECBEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type type, uint Step, uint cost, uint nb = 0);

	virtual ~ECBEntity() {}

/* Constantes */
public:

	/** Use this function to know how people there are when you create this entity */
	virtual uint InitNb() const = 0;

	virtual bool IsBuilding() const { return false; }

	/** Use this function to know if this entity can create an other entity */
	virtual bool CanCreate(const ECBEntity*) = 0;

	/** Qualitatif */
	virtual const char* Qual() const = 0;

	/** Use this function to know if this entity is able to attaq an other entity */
	virtual bool CanAttaq(const ECBEntity* e) = 0;

	/** Use this function when this entity wants to attaq someone */
	virtual bool WantAttaq(uint x, uint y) { return true; }

	/** Use this function when this entity wants to move somewhere */
	virtual bool WantMove(ECBMove::E_Move) { return true; }

	/** Use this function to deploy your entity */
	virtual bool WantDeploy() { return false; }

/* Methodes */
public:

	/** Use this function to know if this entity can be created */
	virtual bool CanBeCreated(ECBCase* c = 0) const;

	virtual bool CanBeCreated(ECBPlayer* pl) const;

	/** Use this function to add some units in the entity */
	virtual bool AddUnits(uint units);

	/** This function remove entity from case where it is and add it in the new case.
	* It changes pointer \a acase to \a new_case too.
	 * @param new_case this is the new case where we send our entity
	 */
	virtual void ChangeCase(ECBCase* new_case);

	/** Use this function when an entity have played. */
	virtual void Played();

	bool Like(ECBEntity* e);

/* Attributs */
public:

	void SetCase(ECBCase* _c) { acase = _c; }
	ECBCase* Case() const { return acase; }

	void SetOwner(ECBPlayer* _p) { owner = _p; }
	ECBPlayer* Owner() const { return owner; }

	const char* ID() const { return name; }

	virtual ECBEntity* Last() const { return 0; }

	uint EventType() const { return event_type; }
	void SetEvent(uint _e) { event_type = _e; }

	e_type Type() const { return type; }

	/** Return the number of soldats in the army */
	uint Nb() const { return nb; }
	void SetNb(uint n) { nb = n; }
	void Shooted(uint n) { shooted += n; }
	void ReleaseShoot() { nb -= (shooted > nb ? nb : shooted); shooted = 0; }

	/** This unit is locked, because it is new, or deleted, or in a move, or in a transport. */
	bool Locked() const { return lock; }
	void SetLock(bool l = true) { lock = l; }
	void Lock(bool b = true) { lock = b; }
	void Unlock() { lock = false; }

	uint MyStep() const { return myStep; }
	void SetMyStep(uint s) { myStep = s; }
	uint RestStep() const { return restStep; }
	void SetRestStep(uint s) { restStep = s; }

	void SetDeployed(bool d = true) { deployed = d; }
	bool Deployed() const { return deployed; }

	uint Cost() const { return cost; }

	std::string LongName();

/* Variables protégées */
protected:
	ECBPlayer* owner;
	Entity_ID name;
	ECBCase *acase;
	e_type type;
	uint nb;
	bool lock;
	bool deployed;
	uint myStep;
	uint restStep;
	uint shooted;
	uint cost;
	uint event_type;
};

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

	ECBCase() : map(0), map_country(0) {}
	ECBCase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id);

	virtual ~ECBCase() {}

/* Methodes */
public:

	virtual bool CanCreate(const ECBEntity*) { return false; }

	void CheckChangingOwner(ECBEntity* e);

/* Attributs */
public:

	/** This case is a member of this map */
	ECBMap* Map() { return map; }

	/** Return type of terrain (a char) */
	char TypeID() { return type_id; }

	/** Return flags of case */
	uint Flags() { return flags; }

	ECBCountry* Country() { return map_country; }
	void SetCountry(ECBCountry *mc);

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
class ECBVille : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBVille(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
	ECBVille() {}

/* Methodes */
public:

	virtual bool CanCreate(const ECBEntity* e)
	{
		switch(e->Type())
		{
			case ECBEntity::E_ARMY:
				return true;
			default:
				return false;
		}
	}

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is a land */
class ECBTerre : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBTerre(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
	ECBTerre() {}

/* Methodes */
public:

	virtual bool CanCreate(const ECBEntity* e) { return (e->IsBuilding()); }

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is sea */
class ECBMer : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBMer(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
	ECBMer() {}

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is a bridge */
class ECBPont : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBPont(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
	ECBPont() {}

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

	ECBCountry(ECBMap* map, const Country_ID ident);
	virtual ~ECBCountry() {}

/* Attributs */
public:

	/** Add case to country */
	void AddCase(ECBCase* _case) { cases.push_back(_case); }

	/** Return country ID (two chars) */
	const char* ID() const { return ident; }

	/** Return Owner of country */
	ECBMapPlayer* Owner() const { return owner; }
	virtual void SetOwner(ECBMapPlayer* mp) { owner = mp; }

	/** Change owner of country.
	 * This function will remove this country from my actual owner list, and will add me
	 * in the list of my new owner. And it will change turnmoney of the two players.
	 * @param mp this is the new map player
	 */
	virtual bool ChangeOwner(ECBMapPlayer* mp);

	std::vector<ECBCase*> Cases() const { return cases; }

	ECBMap* Map() const { return map; }

	uint Flags() const { return flags; }

/* Variables privées */
protected:
	friend void ECBCase::SetCountry(ECBCountry *mc); /* Il touche à mes flags */
	std::vector<ECBCase*> cases;
	ECBMapPlayer *owner;
	Country_ID ident;
	ECBMap *map;
	uint flags;
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
	bool RemoveCountry(ECBCountry* _country, bool use_delete = false);

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

	void Init();      /**< Initialisation, you *HAVE* to call this !! */

	virtual ~ECBMap();

/* Methodes */
public:

	/** Delete all MapPlayers who are not used by a real player */
	void ClearMapPlayers();

	/** Reload */
	void Reload();

	/** Create a case */
	virtual ECBCase* CreateCase(uint x, uint y, char type_id);

/* Attributs */
public:

	/** Name of channel */
	std::string Name() { return name; }

	/** Channel linked to map */
	ECBChannel* Channel() { return chan; }
	void SetChannel(ECBChannel* c) { chan = c; }

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
	ECList<ECBEntity*> *Neutres() { return &neutres; }

	std::vector<std::string> MapFile() { return map_file; }         /**< Return map_file vector */

	ECBDate* Date() { return date; }
	ECBDate* NextDay() { return &(++(*date)); }                     /**< Increment date to next day */

	std::vector<std::string> MapInfos() { return map_infos; }       /**< Map informations */
	
	/** Access to a case of map 
	 * Example: map(x,y)
	 */
	ECBCase* operator() (uint x, uint y) const;

	void RemoveAnEntity(ECBEntity*, bool use_delete = false);
	void AddAnEntity(ECBEntity*);

/* Variables privées */
protected:
	std::vector<ECBCase*> map;              /**< Alignement fait ligne par ligne !!! */

	std::vector<std::string> map_file;
	
	std::vector<ECBMapPlayer*> map_players;
	
	std::vector<ECBCountry*> map_countries;

	std::vector<std::string> map_infos;

	ECList<ECBEntity*> entities;
	ECList<ECBEntity*> neutres;

	ECBDate* date;

	ECBChannel *chan;
	std::string name;

	uint x, y;
	uint min, max;
	uint nb_soldats;

	uint begin_money, city_money;

	bool initialised; /**< This variable is setted to true only when \a map is empty */

	void Destruct();  /**< Free of memory */

	/** This function have to be redefined by client to set attributs of images in case */
	virtual void SetCaseAttr(ECBCase*, char) {}

	virtual ECBCountry* CreateCountry(ECBMap* m, const Country_ID ident) { return new ECBCountry(m, ident); }

	ECBMap();         /**< Disallowed */
};

#endif /* ECLIB_MAP_H */
