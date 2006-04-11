/* src/Map.h - Header of Map.cpp
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

#ifndef EC_MAP_H
#define EC_MAP_H

#include "lib/Map.h"
#include "tools/Images.h"
#include <vector>

class ECase;
class TMap;
class ECEntity;

typedef ECBMapPlayer   ECMapPlayer;
typedef ECBCountry     ECountry;
typedef ECBDate        ECDate;

/********************************************************************************************
 *                                   ECMove                                                 *
 ********************************************************************************************/
class ECMove : public ECBMove
{
/* Constructeur/Destructeur */
public:

	ECMove(ECBEntity* e) : ECBMove(e), dest(0) {}

/* Attributs */
public:

	void AddMove(E_Move m);
	void SetMoves(Vector _moves);
	void RemoveFirst() { if(!moves.empty()) moves.erase(moves.begin()); }
	E_Move First() { return *(moves.begin()); }

	ECBCase* Dest() { return dest; }
	void EstablishDest();

/* Variables protégées */
protected:
	ECBCase* dest;
};

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/
class ECEntity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	ECEntity() : Tag(0), image(0), selected(false), move(this) {}

	ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint _Step, uint _nb = 0);

	virtual ~ECEntity();

/* Informations */
public:

	virtual const char* Name() const = 0;
	virtual const char* Infos() const = 0;
	virtual ECImage* Icon() const = 0;

/* Methodes */
public:

	virtual bool BeforeEvent() = 0;

	virtual bool MakeEvent() = 0;

	virtual bool AfterEvent() = 0;

	virtual void ChangeCase(ECBCase* new_case);

/* Attributs */
public:

	bool Test(int x, int y);

	void Draw();
	ECSprite* Image() const { return image; }
	void SetImage(ECSpriteBase* spr);
	void SetAnim(bool anim = true) { if(image) image->SetAnim(anim); }

	void Select(bool s = true) { selected = s; }
	bool Selected() { return selected; }

	int Tag;

	ECMove* Move() { return &move; }

/* Variables privées */
protected:
	ECSprite* image;
	bool selected;
	ECMove move;
};

/********************************************************************************************
 *                                ECase                                                     *
 ********************************************************************************************/

class ECase : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:

	ECase() : image(0), selected(0) { }

	ECase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id);

	virtual ~ECase();

/* Methodes */
public:

/* Attributs */
public:

	virtual const char* Name() const = 0;

	bool Test(int x, int y);

	void Draw();
	ECSprite* Image() const { return image; }
	void SetImage(ECSpriteBase* spr);

	void Select(bool s = true) { selected = s; }
	bool Selected() const { return selected; }

/* Variables privées */
protected:
	ECSprite* image;
	bool selected;
};

/** This class is a derived class from ECBCase whose is a city */
class ECVille : public ECBVille, public ECase
{
/* Constructeur/Destructeur */
public:
	ECVille(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return flags & C_CAPITALE ? "Capitale" : "Ville"; }

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is a land */
class ECTerre : public ECBTerre, public ECase
{
/* Constructeur/Destructeur */
public:
	ECTerre(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return "Terre"; }

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is sea */
class ECMer : public ECBMer, public ECase
{
/* Constructeur/Destructeur */
public:
	ECMer(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return "Mer"; }

/* Attributs */
public:

/* Variables privées */
protected:

};

/** This class is a derived class from ECBCase whose is a bridge */
class ECPont : public ECBPont, public ECase
{
/* Constructeur/Destructeur */
public:
	ECPont(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return "Pont"; }

/* Attributs */
public:

/* Variables privées */
protected:

};

/********************************************************************************************
 *                               ECMap                                                      *
 ********************************************************************************************/

class ECMap : public ECBMap
{
/* Constructeur/Destructeur */
public:
	/** Path of map (used only by Server) */
	ECMap(std::string _filename);

	/** Constructor from a string's vector
	 * @param _map_file this is a string's vector where there is informations about map
	 */
	ECMap(std::vector<std::string> _map_file);

	virtual ~ECMap();

/* Methodes */
public:

	void CreatePreview(uint width = 200, uint height = 200, bool ingame = false);

	virtual ECBCase* CreateCase(uint x, uint y, char type_id);

/* Attributs */
public:

	ECImage* Preview() { return preview; }            /**< Creation of map preview */

	TMap* ShowMap() { return showmap; }
	void SetShowMap(TMap* sm) { showmap = sm; }

/* Variables privées */
protected:
	ECImage *preview;
	TMap *showmap;

	virtual void SetCaseAttr(ECBCase*, char);
};

/********************************************************************************************
 *                               ECEntityList                                               *
 ********************************************************************************************/

class ECEntityList
{
/* Constructeur/Destructeur */
public:
	ECEntityList();
	~ECEntityList();

/* Attributs */
public:
	std::vector<ECEntity*> List() const { return entities; }

	template<typename T>
	std::vector<ECEntity*> ECEntityList::CanCreatedBy(T c) const
	{
		std::vector<ECEntity*> l;
		if(!c) return l;
		for(std::vector<ECEntity*>::const_iterator it = entities.begin(); it != entities.end(); ++it)
			if(c->CanCreate(*it))
				l.push_back(*it);
		return l;
	}

/* Variables privées */
private:
	std::vector<ECEntity*> entities;
};

extern ECEntityList EntityList;

#endif /* EC_MAP_H */
