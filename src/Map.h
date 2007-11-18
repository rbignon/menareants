/* src/Map.h - Header of Map.cpp
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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
#include "gui/ProgressBar.h"
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
typedef ECBMove        ECMove;

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
 *                                 ECEntity                                                 *
 ********************************************************************************************/
class ECEntity : public virtual ECBEntity
{
/* Constructeur/Destructeur */
public:

	enum imgs_t {
		I_Up=ECMove::Up,
		I_Down=ECMove::Down,
		I_Left=ECMove::Left,
		I_Right=ECMove::Right,
		I_Attaq,
		I_Deployed,
		I_Reployed
	};
	typedef std::map<imgs_t, ECSpriteBase*> ImgList;

	ECEntity();
	ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case);

	virtual ~ECEntity();

/* Informations */
public:

	virtual const char* Name() const = 0;
	virtual const char* Infos() const = 0;
	virtual const char* Description() const = 0;
	virtual ECImage* Icon() const = 0;
	virtual ECSpriteBase* DeadCase() const;
	virtual bool CanBeSelected() const { return true; }
	virtual bool OnTop() const { return false; }
	virtual bool IsHiddenByMe(ECBEntity* e) const { return false; }
	/** Qualitatif */
	virtual const char* Qual() const = 0;

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) = 0;

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) = 0;

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*) = 0;

	virtual void ChangeCase(ECBCase* new_case);

	virtual void SetShowedCases(bool show, bool forced = false);

	virtual void Played();

	virtual void RecvData(ECData) { return; }

	virtual std::string SpecialInfo() { return ""; }

	virtual std::string DeployButton() { return Deployed() ? _("Fold up") : _("Deploy"); }

	virtual void Draw();

	virtual void AfterDraw();

	/** This function is called when this unit is created */
	virtual void Created() {}

	bool CanWalkTo(ECase* c, bool &move, bool &invest);

	virtual void UpdateImages() = 0;

	virtual void Init();

/* Attributs */
public:

	bool IsHiddenOnCase();

	bool Test(int x, int y);

	ECSprite* Image() const { return image; }
	void SetImage(ECSpriteBase* spr);
	void SetAnim(bool anim = true) { if(image) image->SetAnim(anim); }

	ECSprite* AttaqImg() const { return attaq; }
	void SetAttaqImg(ECSpriteBase* spr, int x, int y);

	void Select(bool s = true);
	bool Selected() { return selected; }

	int Tag;

	void ImageSetXY(int x, int y);

	std::string OwnerNick() const;

	ECase* Case() const;
	ECPlayer* Owner() const;

	virtual void SetOwner(ECBPlayer*);


	ECase* AttaquedCase() const { return attaqued_case; }
	void SetAttaquedCase(ECase* c);

	EChannel* Channel() const;
	ECMap* Map() const;

	ECImage* Trajectoire() { return &trajectoire; }

	virtual void SetNb(uint n);
	void SetMaxNb(uint n) { max_nb = n; }

/* Variables privées */
private:
	ECSprite* image;
	ECSprite* attaq;
	ECImage trajectoire;
	TProgressBar life;

protected:
	bool selected;
	ECase* attaqued_case;
	uint max_nb;

	ImgList images;
	void PutImage(imgs_t i, ECSpriteBase* b);

	ECSpriteBase* GetSprite(imgs_t t) { return images[t]; }
};

/********************************************************************************************
 *                                ECase                                                     *
 ********************************************************************************************/

class ECase : public virtual ECBCase
{
/* Constructeur/Destructeur */
public:

	ECase() : image(0), selected(0), showed(-1), must_redraw(true) { }

	ECase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id);

	virtual ~ECase();

/* Attributs */
public:

	virtual const char* Name() const = 0;

	bool Test(int x, int y);

	void Draw();
	ECSprite* Image() const { return image; }
	void SetImage(ECSpriteBase* spr);

	void Select(bool s = true) { selected = s; }
	bool Selected() const { return selected; }

	/** Is this case is showed ?
	 * o -1 = hidden
	 * o 0  = shadowed
	 * o >0 = showed
	 */
	int Showed() const;
	void SetShowed(int s) { showed = s; }
	bool Visible() const;

	bool MustRedraw() const { return must_redraw; }
	void SetMustRedraw(bool b = true) { must_redraw = b; }

/* Variables privées */
protected:
	ECSprite* image;
	bool selected;
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

	virtual const char* Name() const { return _("Ground"); }
};

/** This class is a derived class from ECBCase whose is sea */
class ECMer : public ECBMer, public ECase
{
/* Constructeur/Destructeur */
public:
	ECMer(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return _("Water"); }
};

/** This class is a derived class from ECBCase whose is a bridge */
class ECPont : public ECBPont, public ECase
{
/* Constructeur/Destructeur */
public:
	ECPont(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return _("Bridge"); }
};

/** This class is a derived class from ECBCase whose is a montain */
class ECMontain : public ECBMontain, public ECase
{
/* Constructeur/Destructeur */
public:
	ECMontain(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id) : ECBCase(_map, _x, _y, _flags, _type_id) {}

/* Methodes */
public:

	virtual const char* Name() const { return _("Montain"); }
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
	void CreatePreview(uint width, uint height, int flags);

	virtual ECBCase* CreateCase(uint x, uint y, char type_id);

	static bool CanSelect(ECBCase* c);

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

	std::string ShowWaitMessage;

/* Variables privées */
private:
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

	ECEntity* Get(ECEntity::e_type i) const { return entities[i-1]; }

	std::vector<ECEntity*> CanAttaq(ECBEntity* c) const
	{
		std::vector<ECEntity*> l;
		if(!c) return l;
		for(std::vector<ECEntity*>::const_iterator it = entities.begin(); it != entities.end(); ++it)
			if(c->CanAttaq(*it))
				l.push_back(*it);
		return l;
	}

	std::vector<ECEntity*> CanInvest(ECBEntity* c) const
	{
		std::vector<ECEntity*> l;
		if(!c) return l;
		for(std::vector<ECEntity*>::const_iterator it = entities.begin(); it != entities.end(); ++it)
			if(c->CanInvest(*it))
				l.push_back(*it);
		return l;
	}

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
