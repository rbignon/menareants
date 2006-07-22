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

struct ECData
{
	ECData(int = 0) : type(0) {}

	ECData(int _t, std::string _d) : type(_t), data(_d) {}

	int type;
	std::string data;
};

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

	bool Empty() const { return moves.empty(); }                    ///< There isn't movements ?
	Vector::size_type Size() const { return moves.size(); }         ///< Number of movements
	Vector Moves() const { return moves; }                          ///< Get movements' vector
	void AddMove(E_Move m) { moves.push_back(m); }                  ///< Add a movement as a E_Move enumerator
	void SetMoves(Vector _moves) { moves = _moves; }                ///< Define a movement's vector. This remove actuals moves.
	void SetEntity(ECBEntity* et) { entity = et; }                  ///< Set an entity
	ECBEntity* Entity() { return entity; }

	ECBCase* FirstCase() { return first_case; }                     ///< This is the first case, where the movement begin
	void SetFirstCase(ECBCase* c) { first_case = c; }               ///< Define the first case

	/** Put all movements in a string.
	 * @param end is setted, this is the last case of movement
	 */
	std::string MovesString(ECBCase* end = 0);

	virtual void Clear(ECBCase* c = 0) { moves.clear(); first_case = c; }   ///< Clear all movements
	void Return(ECBCase*);                                                  ///< Remove all movements after a case

/* Variables privées */
protected:
	Vector moves;
	ECBCase* first_case;
	ECBEntity* entity;
};

/********************************************************************************************
 *                               ECBEntity                                                  *
 ********************************************************************************************/
/* Documentation is in server/Channels.h:EChannel::SendArm() */
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
#define ARM_FORCEATTAQ  0x1000
#define ARM_CONTENER    0x2000
#define ARM_UNCONTENER  0x4000
#define ARM_NOPRINCIPAL 0x8000
#define ARM_DATA        0x10000
#define ARM_UPGRADE     0x20000
#define ARM_CHANGEOWNER 0x40000
#define ARM_PREUNION    (ARM_MOVE|ARM_LOCK)
#define ARM_UNION       (ARM_MOVE|ARM_REMOVE)
#define ARM_CREATE      (ARM_MOVE|ARM_TYPE|ARM_NUMBER)
#define ARM_CONTAIN     (ARM_CONTENER|ARM_MOVE)
#define ARM_UNCONTAIN   (ARM_UNCONTENER|ARM_MOVE)
/* Concerne server:EChannel::SendArm() */
#define ARM_HIDE        0x0100 /* cache les infos aux users non concernés */
#define ARM_RECURSE     0x0200 /* ne JAMAIS appeler */
#define ARM_NOCONCERNED 0x0400 /* ne pas envoyer aux users concernés */

#define ENTITY_EMPTY_CONSTRUCTOR(x)  x()
#define ENTITY_CONSTRUCTOR(x)        x(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case) \
                                     :  ECBEntity(_name, _owner, _case)
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
		E_CITY,
		E_CAPITALE,
		E_SHIPYARD,
		E_BOAT,
		E_NUCLEARSEARCH,
		E_SILO,
		E_ENGINER,
		E_DEFENSETOWER,
		E_TOURIST,
		E_MINE,
		E_END
	};

	ECBEntity()
		: owner(0), acase(0), nb(0), lock(false), deployed(false), myStep(0), restStep(0), event_type(0),
		  parent(0), map(0)
	{}

	ECBEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case);

	virtual ~ECBEntity() {}

/* Constantes */
public:

	/** This is the type of entity */
	virtual e_type Type() const = 0;

	/** Porty of the entity */
	virtual uint Porty() const { return 0; }

	/** This is the price of this entity */
	virtual uint Cost() const = 0;

	/** Use this function to know how people there are when you create this entity */
	virtual uint InitNb() const = 0;

	/** This is the visibility of this entity (rayon) */
	virtual uint Visibility() const { return 3; }

	/** Initial step */
	virtual uint Step() const { return 0; }

	virtual bool IsCountryMaker() const { return false; }    /**< Is it an entity who sets the country's owner ? */
	virtual bool IsCity() const { return false; }            /**< This building is a part of a city */
	virtual bool IsBuilding() const { return false; }
	virtual bool IsNaval() const { return false; }
	virtual bool IsInfantry() const { return false; }
	virtual bool IsVehicule() const { return false; }
	virtual bool IsHidden() const { return false; }          /**< This unit can be views only by his owner and his allies */

	/** Use this function to know if this entity can create an other entity */
	virtual bool CanCreate(const ECBEntity*) = 0;

	/** This function check if this unit can be created by any nation.
	 * Default: unit can be created by all nations
	 */
	virtual bool CanBeCreated(uint nation) const { return true; }

	virtual bool CanContain(const ECBEntity*) { return false; }

	virtual bool CanInvest(const ECBEntity* e) const
	{
		if(Like(e)) return false;
		switch(e->Type())
		{
			case E_CITY:
			case E_CAPITALE:
				return true;
			default:
				return false;
		}
	}

	/** Qualitatif */
	virtual const char* Qual() const = 0;

	/** Use this function to know if this entity is able to attaq an other entity */
	virtual bool CanAttaq(const ECBEntity* e) = 0;

	/** Use this function when this entity wants to attaq someone */
	virtual bool WantAttaq(uint x, uint y, bool force = false) { return true; }

	#define MOVE_FORCE 0x001
	#define MOVE_SIMULE 0x002
	virtual bool WantMove(ECBMove::E_Move, int flags = 0) { return true; } ///< Does this entity wants to move somewhere ?

	/** This is my upgrade (with ARM_UPGRADE/°) */
	virtual e_type MyUpgrade() const { return E_NONE; }

	/** Use this function to deploy your entity */
	virtual bool WantDeploy() { return false; }

	/** If this entity add money, use this function to return how many money it will add or remove */
	virtual int TurnMoney() { return 0; }

/* Methodes */
public:

	/** Must be called ONLY in server ! */
	virtual void Invest(ECBEntity* e);

	/** This virtual function must be redefined in server to change owner, but it called by ECBCase::CheckChaningOwner() */
	virtual void ChangeOwner(ECBPlayer*) { assert(0); }

	/** Use this function to know if this entity can be created */
	virtual bool CanBeCreated(ECBCase* c = 0) const;

	/** Use this function to know if this entity can be created <b>by this player</b> (about nations)*/
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

	virtual void Init();

	/** Does this entity like an other entity ? */
	bool Like(const ECBEntity* e) const;

/* Attributs */
public:

	/** This is the case where this entity is */
	ECBCase* Case() const { return acase; }
	void SetCase(ECBCase* _c) { acase = _c; }

	/** Owner of this entity (return a \a player !) */
	ECBPlayer* Owner() const { return owner; }
	void SetOwner(ECBPlayer* _p) { owner = _p; }

	/** This is an identificator of this entity (xy, x ¤ [A..z], y ¤ [A..z]) */
	const char* ID() const { return name; }
	void SetID(const char* s);

	/** Return a long identificator in form Owner!xy (x ¤ [A..z], y ¤ [A..z]) */
	std::string LongName();

	/** This is used by server (so, why is it here ??) */
	virtual ECBEntity* Last() const { return 0; }

	/** This is events of this entity */
	uint EventType() const { return event_type; }
	void SetEvent(uint _e) { event_type = _e; }
	void AddEvent(uint _e) { event_type |= _e; }
	void DelEvent(uint _e) { event_type &= ~(_e); }

	/** Return the number of soldats in the army */
	uint Nb() const { return nb; }
	virtual uint RealNb() const { return Nb(); }
	void SetNb(uint n) { nb = n; }

	/** This unit is locked, because it is new, or deleted, or in a move, or in a transport. */
	bool Locked() const { return lock; }
	void SetLock(bool l = true) { lock = l; }
	void Lock(bool b = true) { lock = b; }
	void Unlock() { lock = false; }

	/** This is my maximum steps */
	uint MyStep() const { return myStep; }
	void SetMyStep(uint s) { myStep = s; }

	/** This is my rest steps */
	uint RestStep() const { return restStep; }
	void SetRestStep(uint s) { restStep = s; }

	/** Is this entity deployed ?
	 * An entity can be deployed to use a special attaq, for example a Missile Launcher must be deployed to
	 * shoot someone.
	 */
	bool Deployed() const { return deployed; }
	void SetDeployed(bool d = true) { deployed = d; }

	/** C'est pour que ECList\<ECBEntity*\> puisse faire appel à Shadowed(), qui ne sera utilisé que dans le serveur.
	 * En effet ce dernier a que des instances de ECEntity qui eux contiennent bien Shadowed().
	 */
	virtual bool Shadowed() const { return false; }

	ECBEntity* Parent() const { return parent; }
	void SetParent(ECBEntity* e) { parent = e; }

	ECBMap* Map() const { return map; }
	void SetMap(ECBMap* m) { map = m; }

/* Variables protégées */
protected:
	ECBPlayer* owner;
	Entity_ID name;
	ECBCase *acase;
	uint nb;
	bool lock;
	bool deployed;
	uint myStep;
	uint restStep;
	uint event_type;
	ECBEntity* parent;
	ECBMap* map;
};

/********************************************************************************************
 *                               ECBDate                                                    *
 ********************************************************************************************/

class ECBDate
{
/* Constructeur/Destructeur */
public:
	ECBDate();
	ECBDate(uint d, uint m, int y);
	ECBDate(std::string date);

/* Methodes */
public:

	std::string String();

	/** Check if this date is correct */
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

	void SetDate(uint days);

	ECBDate& operator++ ();    ///< prefix ++
	ECBDate  operator++ (int); ///< postfix ++

/* Variables privées */
protected:
	uint d;
	uint m;
	int y;
};

/********************************************************************************************
 *                               ECBCase                                                    *
 ********************************************************************************************/

#define C_TERRE          0x0001
#define C_MER            0x0002
#define C_PONT           0x0004

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

	/** Calculate distance */
	uint Delta(ECBCase* c);

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

/** This class is a derived class from ECBCase whose is a land */
class ECBTerre : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:
	ECBTerre(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
	ECBTerre() {}

/* Methodes */
public:

	/** ECBTerre can create all buildings */
	virtual bool CanCreate(const ECBEntity* e) { return (e->IsBuilding() && !e->IsNaval()); }

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

	virtual bool CanCreate(const ECBEntity* e) { return (e->IsBuilding() && e->IsNaval()); }

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
	void RemoveCase(ECBCase* _case);

	/** Return country ID (two chars) */
	const char* ID() const { return ident; }

	/** Return Owner of country */
	ECBMapPlayer* Owner() const { return owner; }
	virtual void SetOwner(ECBMapPlayer* mp) { owner = mp; }

	/** Change owner of country.
	 * This function will remove this country from my actual owner list, and will add me
	 * in the list of my new owner.
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

	virtual ~ECBMapPlayer() {}

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

	/** This is all entity who will be created by server when the game will start.
	 * For now, this is only in text format
	 */
	std::vector<std::string> Units() const { return units; }
	void AddUnit(std::string s) { units.push_back(s); }

/* Variables privées */
protected:
	MapPlayer_ID id;
	uint num;
	ECBPlayer* pl;
	std::vector<ECBCountry*> countries;
	std::vector<std::string> units;

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
	std::string& Name() { return name; }

	/** Channel linked to map */
	ECBChannel* Channel() { return chan; }
	void SetChannel(ECBChannel* c) { chan = c; }

	int& CityMoney() { return city_money; }            ///< This is money that is given to players at each turn by cities
	uint& MinPlayers() { return min; }                 /**< Min players to play */
	uint& MaxPlayers() { return max; }                 /**< Max players to play */
	uint NbCases() { return map.size(); }              /**< Number of cases */
	uint NbMapPlayers() { return map_players.size(); } /**< Number of map players */
	uint NbCountries() { return map_countries.size(); }/**< Number of map countries */

	uint Width() { return x; }
	uint Height() { return y; }

	BCaseVector Cases() { return map; }                             /**< Return case vector */
	BMapPlayersVector MapPlayers() { return map_players; }          /**< Return map players vector */
	BCountriesVector Countries() { return map_countries; }          /**< Return countries vector */

	ECList<ECBEntity*> *Entities() { return &entities; }            ///< All entities of game (with neutrales entities)
	ECList<ECBEntity*> *Neutres() { return &neutres; }              ///< All neutrales entities

	std::vector<std::string> MapFile() { return map_file; }         /**< Return map_file vector */

	ECBDate* Date() { return &date; }                               ///< Date of day
	ECBDate* NextDay() { return &(++date); }                        /**< Increment date to next day */

	std::vector<std::string> MapInfos() { return map_infos; }       /**< Map informations */
	
	/** Access to a case of map 
	 * example: map(x,y)
	 */
	ECBCase*& operator() (uint x, uint y);

	void RemoveAnEntity(ECBEntity*, bool use_delete = false);
	void AddAnEntity(ECBEntity*);

	/** There are all neutrales units in text format. See ECBMapPlayer::Units()'s documentation */
	std::vector<std::string> NeutralUnits() const { return neutres_units; }
	void AddNeutralUnit(std::string s) { neutres_units.push_back(s); }

/* Variables privées */
protected:
	std::vector<ECBCase*> map;              /**< Alignement fait ligne par ligne !!! */

	std::vector<std::string> map_file;
	
	std::vector<ECBMapPlayer*> map_players;
	
	std::vector<ECBCountry*> map_countries;

	std::vector<std::string> map_infos;

	ECList<ECBEntity*> entities;
	ECList<ECBEntity*> neutres;

	std::vector<std::string> neutres_units;

	ECBDate date;

	ECBChannel *chan;
	std::string name;
	std::string filename;

	uint x, y;
	uint min, max;

	int city_money;

	bool initialised; /**< This variable is setted to true only when \a map is empty */

	void Destruct();  /**< Free of memory */

	/** This function have to be redefined by client to set attributs of images in case */
	virtual void SetCaseAttr(ECBCase*, char) {}

	virtual ECBCountry* CreateCountry(ECBMap* m, const Country_ID ident) { return new ECBCountry(m, ident); }

	virtual ECBMapPlayer* CreateMapPlayer(char _id, uint _num) { return new ECBMapPlayer(_id, _num); }
	virtual void VirtualAddUnit(std::string line) { return; }

	ECBMap();         /**< Disallowed */
};

#endif /* ECLIB_MAP_H */
