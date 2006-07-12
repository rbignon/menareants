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
class EChannel;
class ECEntity;
class EC_Client;
class Color;
class ECPlayer;
class ECMap;

struct case_img_t
{
	char c;
	ECSpriteBase* (*spr) ();
	char type;
};
extern struct case_img_t case_img[];

typedef ECBMapPlayer   ECMapPlayer;
typedef ECBCountry     ECountry;
typedef ECBDate        ECDate;

class ECMissile
{
/* Constructeur/Destructeur */
public:

	ECMissile(ECEntity* et, int _step)
		: missile(0), missile_up(0), missile_down(0), entity(et), step(_step)
	{}

/* Methodes */
public:

	bool AttaqFirst(ECase* c, EC_Client* me);
	bool AttaqSecond(ECase* c, EC_Client* me);

	void Draw();

/* Attributs */
public:

	ECEntity* Entity() const { return entity; }

	void SetMissileUp(ECSpriteBase* spr) { missile_up = spr; }
	void SetMissileDown(ECSpriteBase* spr) { missile_down = spr; }

	void SetXY(int x, int y);

	ECSprite* Missile() const { return missile; }

/* Variables privées */
private:
	ECSprite* missile;
	ECSpriteBase* missile_up;
	ECSpriteBase* missile_down;
	ECEntity* entity;
	int step;

	void SetMissile(ECSpriteBase* c);
};

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
	void Clear(ECBCase* c) { ECBMove::Clear(c); dest = 0; }

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

	ECEntity()
		: Tag(0), image(0), attaq(0), selected(false), move(this), want_deploy(false), attaqued_case(0)
	{}

	ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint _Step, uint _nb = 0,
	         uint _visibility = 3);

	virtual ~ECEntity();

/* Informations */
public:

	virtual const char* Name() const = 0;
	virtual const char* Infos() const = 0;
	virtual ECImage* Icon() const = 0;

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) = 0;

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) = 0;

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) = 0;

	virtual void ChangeCase(ECBCase* new_case);

	virtual void RefreshColor(Color) = 0;

	virtual void SetShowedCases(bool show, bool forced = false);

	virtual void Played();

	virtual void RecvData(ECData) { return; }

	virtual std::string SpecialInfo() { return ""; }

/* Attributs */
public:

	bool Test(int x, int y);

	virtual void Draw();
	ECSprite* Image() const { return image; }
	void SetImage(ECSpriteBase* spr);
	void SetAnim(bool anim = true) { if(image) image->SetAnim(anim); }

	ECSprite* AttaqImg() const { return attaq; }
	void SetAttaqImg(ECSpriteBase* spr, int x, int y);

	void Select(bool s = true);
	bool Selected() { return selected; }

	int Tag;

	void ImageSetXY(int x, int y);

	ECase* Case() const;
	ECPlayer* Owner() const;

	ECMove* Move() { return &move; }

	bool IWantDeploy() const { return want_deploy; }
	void SetWantDeploy(bool b = true) { want_deploy = b; }

	ECase* AttaquedCase() const { return attaqued_case; }
	void SetAttaquedCase(ECase* c);

	EChannel* Channel() const;
	ECMap* Map() const;

/* Variables privées */
private:
	ECSprite* image;
	ECSprite* attaq;
	ECImage trajectoire;

protected:
	bool selected;
	ECMove move;
	bool want_deploy;
	ECase* attaqued_case;
};

/********************************************************************************************
 *                                ECase                                                     *
 ********************************************************************************************/

class ECase : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:

	ECase() : image(0), selected(0), img_id(0), showed(-1), must_redraw(true) { }

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

	void SetImgID(char id) { img_id = id; }
	char ImgID() const { return img_id; }

	/** Is this case is showed ?
	 * o -1 = hidden
	 * o 0  = shadowed
	 * o >0 = showed
	 */
	int Showed() const { return showed; }
	void SetShowed(uint s) { showed = s; }
	bool Visible() const { return (showed > 0); }

	bool MustRedraw() const { return must_redraw; }
	void SetMustRedraw(bool b = true) { must_redraw = b; }

/* Variables privées */
protected:
	ECSprite* image;
	bool selected;
	char img_id;
	int showed;
	bool must_redraw;
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

/* Methodes */
public:

	#define P_FRONTMER  0x001
	#define P_ENTITIES  0x002
	#define P_POSITIONS 0x004
	void CreatePreview(uint width, uint height, int flags);

	virtual ECBCase* CreateCase(uint x, uint y, char type_id);

/* Attributs */
public:

	ECImage* Preview() { return &preview; }            /**< Creation of map preview */

	TMap* ShowMap() { return showmap; }
	void SetShowMap(TMap* sm) { showmap = sm; }

	virtual void SetCaseAttr(ECBCase*, char);

	bool Brouillard() const { return brouillard; }
	void SetBrouillard(bool b = true) { brouillard = b; }

	uint PixelSize() const { return pixel_size; }

	uint& NbDays() { return nb_days; }
	ECDate* InitDate() { return &init_date; }

/* Variables privées */
protected:
	ECImage preview;
	TMap *showmap;
	ECDate init_date;
	bool brouillard;
	uint pixel_size;
	uint nb_days;
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
	std::vector<ECEntity*> CanCreatedBy(T c, ECBPlayer* pl) const
	{
		std::vector<ECEntity*> l;
		if(!c) return l;
		for(std::vector<ECEntity*>::const_iterator it = entities.begin(); it != entities.end(); ++it)
			if(c->CanCreate(*it) && (*it)->CanBeCreated(pl))
				l.push_back(*it);
		return l;
	}

	std::vector<ECEntity*> Buildings(ECBPlayer* pl = 0) const
	{
		std::vector<ECEntity*> l;
		for(std::vector<ECEntity*>::const_iterator it = entities.begin(); it != entities.end(); ++it)
			if((*it)->IsBuilding() && (!pl || (*it)->CanBeCreated(pl)))
				l.push_back(*it);
		return l;
	}

/* Variables privées */
private:
	std::vector<ECEntity*> entities;
};

extern ECEntityList EntityList;

#endif /* EC_MAP_H */
