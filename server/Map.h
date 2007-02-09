/* server/Map.h - Header of Map.cpp
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

#ifndef ECD_MAP_H
#define ECD_MAP_H

#include "lib/Map.h"
#include <set>

/* Note: il n'y a theoriquement aucune redefinition des ECB* pour les classes suivantes car toutes
 * les informations y sont déjà incluses.
 * C'est pourquoi pour les types qui n'ont pas été à redéfinir il y a des typedef pour les noms courants.
 */

typedef ECBMapPlayer   ECMapPlayer;
typedef ECBDate        ECDate;
typedef ECBMove        ECMove;
class EChannel;
class ECPlayer;
class TClient;
class ECEvent;
class ECase;

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/
class ECEntity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ECEntity() : Tag(0), shooted(0), zombie(false) {}

	ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case)
		: ECBEntity(_name, _owner, _case), Tag(0), shooted(0), zombie(false)
	{}

	virtual ~ECEntity();

/* Methodes */
public:

	/** Use this function to cancel an action of this entity */
	virtual bool Return();

	/** Use this function to make an union with an other entity */
	virtual void Union(ECEntity*);

	/** Par default, si c'est un CountryMake change l'owner de la country, et ensuite l'owner de l'unité investie */
	virtual void Invest(ECBEntity* e);

	virtual void ChangeOwner(ECBPlayer*);

	/** This function is a default method used by entities to attaq someone.
	 * An entity can redefine this method to shoot others entities as it wants
	 * @param entities this is a vector of all entities who are included in the attaq
	 * @param event this is the event's pointer
	 * @return true if this function must be called an other time after that
	 */
	virtual bool Attaq(std::vector<ECEntity*> entities, ECEvent* event);

	/** This \a static method will check if the entities in list are friends */
	static bool AreFriends(std::vector<ECEntity*> list);

	/** Use this function when an entity have played. */
	virtual void Played();

	/** This unit was created.
	 * It is call when a client creates it in PLAYING (with for_me = true)
	 * and when it is propagate to other players in ANIMING (with for_me = false)
	 */
	virtual void Created(bool for_me) {}

	/** Shoot an entity */
	void Shoot(ECEntity*, uint);
	void Shooted(uint n) { shooted += n; }
	virtual void ReleaseShoot();

	/** Cancel all events and send a message to client */
	void CancelEvents();

	/** This function return a vector of entities who will be attaqued by me if I want to attaq this case
	 * @param c attaqued case
	 * @return a vector of entities I will attaq
	 */
	virtual std::vector<ECBEntity*> GetAttaquedEntities(ECBCase* c) const { return c->Entities()->List(); }

	/** Get an array of owners of there entities */
	static std::nrvector<TClient*> EntitiesToClients(std::vector<ECEntity*>);

/* Attributs */
public:

	/** This is a temporary integer */
	int Tag;

	EChannel* Channel() const;

	ECPlayer* Owner() const;

	void SetZombie(bool b = true) { zombie = b; }
	bool IsZombie() const { return zombie; }

	ECList<ECEvent*>* Events() { return &events; }

/* Variables privées */
protected:
	ECList<ECEvent*> events;
	uint shooted;
	bool zombie;
};

/********************************************************************************************
 *                                 ECEvent                                                  *
 ********************************************************************************************/
/** @page ECEvent_page An ECEvent's description
 *
 *  @todo do this page
 */
/** This is a class to describe an event maked by an or a few entity(ies). */
class ECEvent
{
/* Constructeur/Destructeur */
public:

	ECEvent(uint _f, ECEntity* _entity, ECBCase* _c = 0)
		: acase(_c), flags(_f), nb(0), type(0), entity(_entity)
	{}

/* Methodes */
public:


/* Attributs */
public:

	/** C'est le type de l'evenement */
	uint Flags() const { return flags; }
	void SetFlags(uint _f) { flags = _f; }

	/** Entitées associées. Il y en a qu'une seule *SAUF* pour les attaques */
	ECList<ECEntity*> *Entities() { return &entities; }

	/** Set entity owner */
	ECEntity* Entity() const { return entity; }
	void SetEntity(ECEntity* e) { entity = e; }

	/** Case associée */
	ECBCase* Case() const { return acase; }
	void SetCase(ECBCase* c) { acase = c; }

	/** Si c'est une création, une union ou autre, mettre le nombre */
	void SetNb(uint n) { nb = n; }
	uint Nb() { return nb; }

	/** CE N'EST PAS LE TYPE DE L'ÉVENEMENT, MAIS LE TYPE ASSOCIÉ À UNE CREATION D'ENTITÉ */
	void SetType(uint t) { type = t; }
	uint Type() { return type; }

	/** C'est utilisé pour les MOUVEMENTS */
	ECMove* Move() { return &move; }

	/** C'est utilisé pour les ATTAQUES, on link les evenements de mouvement si il y a */
	std::vector<ECEvent*> Linked() const { return linked; }
	void AddLinked(ECEvent* li) { linked.push_back(li); }
	bool RemoveLinked(ECEvent* li, bool use_delete = false);

/* Variables privées */
protected:
	ECBCase* acase;
	uint flags;
	uint nb;
	uint type;
	ECEntity* entity;
	ECList<ECEntity*> entities;
	ECMove move;
	std::vector<ECEvent*> linked;
};

typedef std::vector<ECEvent*> EventVector;
//typedef std::set<ECEvent*, SortEventsFunction> EventVector;

/********************************************************************************************
 *                                 ECase                                                    *
 ********************************************************************************************/
class ECase : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:

	ECase() {}

	ECase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id)
		: ECBCase(_map, _x, _y, _flags, _type_id)
	{}

/* Methodes */
public:

	void CheckInvests(ECBEntity* e);

};

/** This class is a derived class from ECBCase whose is a land */
class ECTerre : public ECBTerre, public ECase
{
/* Constructeur/Destructeur */
public:
	ECTerre(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
};

/** This class is a derived class from ECBCase whose is sea */
class ECMer : public ECBMer, public ECase
{
/* Constructeur/Destructeur */
public:
	ECMer(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
};

/** This class is a derived class from ECBCase whose is a bridge */
class ECPont : public ECBPont, public ECase
{
/* Constructeur/Destructeur */
public:
	ECPont(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
};

/** This class is a derived class from ECBCase whose is a montain */
class ECMontain : public ECBMontain, public ECase
{
/* Constructeur/Destructeur */
public:
	ECMontain(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}
};

/********************************************************************************************
 *                                 ECountry                                                 *
 ********************************************************************************************/
class ECountry : public ECBCountry
{
/* Constructeur/Destructeur */
public:
	ECountry(ECBMap* map, const Country_ID ident) : ECBCountry(map, ident) {}

/* Methodes */
public:
	bool ChangeOwner(ECBMapPlayer* mp);
};


/********************************************************************************************
 *                                 ECMap                                                    *
 ********************************************************************************************/
class ECMap : public ECBMap
{
/* Constructeur/Destructeur */
public:
	ECMap(std::string _filename)
		: ECBMap(_filename)
	{}

	ECMap(std::vector<std::string> _map_file, uint _i)
		: ECBMap(_map_file), i(_i)
	{}

	virtual ~ECMap();

/* Attributs */
public:

	/** Number in the map list of server */
	uint Num() { return i; }

	/** All events of this map */
	EventVector& Events() { return map_events; }
	void AddEvent(ECEvent* _e) { map_events.push_back(_e); }
	EventVector::iterator RemoveEvent(ECEvent* _e, bool use_delete = false);
	ECEvent* FindEvent(ECBCase*, uint, ECEntity* = 0);

	virtual void RemoveAnEntity(ECBEntity*, bool use_delete = false);

/* Méthodes */
public:

	static bool LoadMaps();

	virtual ECBCase* CreateCase(uint _x, uint _y, char type_id);

/* Variables privées */
protected:
	EventVector map_events;
	uint i;
	virtual ECBCountry* CreateCountry(ECBMap* m, const Country_ID ident) { return new ECountry(m, ident); }
};

typedef std::vector<ECMap*> MapVector;

extern MapVector MapList;
extern MapVector MissionList;

#endif /* ECD_MAP_H */
