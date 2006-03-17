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
typedef ECBCountry     ECountry;
typedef ECBDate        ECDate;

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/
class ECEntity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ECEntity() : Tag(0), event_type(0) {}

	ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint _Step, uint _nb = 0)
		: ECBEntity(_name, _owner, _case, _type, _Step, _nb), Tag(0), event_type(0)
	{}

	virtual ~ECEntity() { }

/* Methodes */
public:

	virtual bool Attaq(std::vector<ECEntity*> entities) = 0;

	static bool AreFriends(std::vector<ECEntity*> list);

/* Attributs */
public:

	uint EventType() { return event_type; }
	void SetEvent(uint _e) { event_type = _e; }

	int Tag;

/* Variables privées */
protected:
	uint event_type;
};

/********************************************************************************************
 *                                 ECEvent                                                  *
 ********************************************************************************************/
#define ARM_MOVE      0x001
#define ARM_SPLIT     0x002
#define ARM_ATTAQ     0x004
#define ARM_REMOVE    0x008
#define ARM_LOCK      0x010
#define ARM_TYPE      0x020
#define ARM_NUMBER    0x040
#define ARM_RETURN    0x080
#define ARM_HIDE      0x100
#define ARM_RECURSE   0x200 /* ne JAMAIS appeler */
#define ARM_UNION     (ARM_MOVE|ARM_NUMBER)
#define ARM_CREATE    (ARM_MOVE|ARM_TYPE|ARM_NUMBER)
class ECEvent
{
/* Constructeur/Destructeur */
public:

	ECEvent(uint _f, ECase* _c = 0)
		: acase(_c), flags(_f), nb(0), type(0)
	{}

/* Methodes */
public:

/* Attributs */
public:

	uint Flags() const { return flags; }
	void SetFlags(uint _f) { flags = _f; }

	ECList<ECEntity*> *Entities() { return &entities; }

	bool operator<(const ECEvent& e) const;

	ECase* Case() const { return acase; }

	void SetNb(uint n) { nb = n; }
	uint Nb() { return nb; }

	void SetType(uint t) { type = t; }
	uint Type() { return type; }

/* Variables privées */
protected:
	ECase* acase;
	uint flags;
	uint nb;
	uint type;
	ECList<ECEntity*> entities;
};

typedef std::vector<ECEvent*> EventVector;
//typedef std::set<ECEvent*, SortEventsFunction> EventVector;

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
	EventVector::iterator RemoveEvent(ECEvent* _e, bool use_delete);

/* Méthodes */
public:

	void RemoveAnEntity(ECBEntity*, bool use_delete = false);
	void AddAnEntity(ECBEntity*);

	void SortEvents();

/* Variables privées */
protected:
	EventVector map_events;
	uint i;
};

extern bool LoadMaps();

typedef std::vector<ECMap*> MapVector;

extern MapVector MapList;

#endif /* ECD_MAP_H */
