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

typedef ECBCase        ECase;
typedef ECBVille       ECVille;
typedef ECBMer         ECMer;
typedef ECBTerre       ECTerre;
typedef ECBPont        ECPont;
typedef ECBMapPlayer   ECMapPlayer;
typedef ECBDate        ECDate;
typedef ECBMove        ECMove;

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/
class ECEntity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ECEntity() : Tag(0), last(0), next(0), move(this) {}

	ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint _Step, uint _nb = 0)
		: ECBEntity(_name, _owner, _case, _type, _Step, _nb), Tag(0), last(0), next(0), move(this)
	{}

	virtual ~ECEntity();

/* Methodes */
public:

	/** Use this function to cancel an action of this entity */
	virtual bool Return(ECBCase* c = 0);

	/** Use this function to create a last state of this entity (stocked in a variable) */
	virtual void CreateLast() = 0;

	/** Use this function to make an union with an other entity */
	virtual void Union(ECEntity*);

	virtual bool Attaq(std::vector<ECEntity*> entities);

	static bool AreFriends(std::vector<ECEntity*> list);

	/** Use this function when an entity have played. */
	virtual void Played();

/* Attributs */
public:

	/** Return last entity */
	ECEntity* Last() const { return last; }
	virtual void RemoveLast();

	ECEntity* FindLast(ECBCase*);
	ECEntity* FindNext();
	ECEntity* Next() const { return next; }

	int Tag;

	ECMove* Move() { return &move; }

/* Variables privées */
protected:
	ECEntity* last;
	ECEntity* next;
	bool SetLast(ECEntity* e) { return (last = e); }
	ECMove move;
};

/********************************************************************************************
 *                                 ECEvent                                                  *
 ********************************************************************************************/
/** @page ECEvent_page An ECEvent's description
 *
 */
/** This is a class to describe an event maked by an or a few entity(ies). */
class ECEvent
{
/* Constructeur/Destructeur */
public:

	ECEvent(uint _f, ECase* _c = 0)
		: acase(_c), flags(_f), nb(0), type(0)
	{}

/* Methodes */
public:

	bool CheckRemoveBecauseOfPartOfAttaqEntity(ECEntity*);

/* Attributs */
public:

	/** C'est le type de l'evenement */
	uint Flags() const { return flags; }
	void SetFlags(uint _f) { flags = _f; }

	/** Entitées associées. Il y en a qu'une seule *SAUF* pour les attaques */
	ECList<ECEntity*> *Entities() { return &entities; }
	ECEntity* Entity() { return entities.First(); }

	/** Permet de comparer la priorité */
	bool operator<(const ECEvent& e) const;

	/** Case associée */
	ECase* Case() const { return acase; }
	void SetCase(ECase* c) { acase = c; }

	/** Si c'est une création, une union ou autre, mettre le nombre */
	void SetNb(uint n) { nb = n; }
	uint Nb() { return nb; }

	/** CE N'EST PAS LE TYPE DE L'ÉVENEMENT, MAIS LE TYPE ASSOCIÉ À UNE CREATION D'ENTITÉ */
	void SetType(uint t) { type = t; }
	uint Type() { return type; }

	/** C'est utilisé pour les MOUVEMENTS */
	ECMove* Move() { return &move; }

	/** C'est utilisé pour les ATTAQUES, on link les evenements de mouvement si il y a */
	std::vector<ECEvent*> Linked() { return linked; }
	void AddLinked(ECEvent* li) { linked.push_back(li); }
	bool RemoveLinked(ECEvent* li, bool use_delete = false);

/* Variables privées */
protected:
	ECase* acase;
	uint flags;
	uint nb;
	uint type;
	ECList<ECEntity*> entities;
	ECMove move;
	std::vector<ECEvent*> linked;
};

typedef std::vector<ECEvent*> EventVector;
//typedef std::set<ECEvent*, SortEventsFunction> EventVector;

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
	ECMap(std::string _filename, uint _i)
		: ECBMap(_filename), i(_i)
	{}

	ECMap(std::vector<std::string> _map_file, uint _i)
		: ECBMap(_map_file), i(_i)
	{}

	virtual ~ECMap();

/* Attributs */
public:

	uint Num() { return i; }

	EventVector Events() const { return map_events; }
	void AddEvent(ECEvent* _e) { map_events.push_back(_e); }
	EventVector::iterator RemoveEvent(ECEvent* _e, bool use_delete = false);
	ECEvent* FindEvent(ECase*, uint, ECEntity* = 0);

/* Méthodes */
public:

	void SortEvents();

/* Variables privées */
protected:
	EventVector map_events;
	uint i;
	virtual ECBCountry* CreateCountry(ECBMap* m, const Country_ID ident) { return new ECountry(m, ident); }
};

extern bool LoadMaps();

typedef std::vector<ECMap*> MapVector;

extern MapVector MapList;

#endif /* ECD_MAP_H */
