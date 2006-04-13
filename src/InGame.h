/* src/InGame.h - Header of InGame.cpp
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

#ifndef EC_INGAME_H
#define EC_INGAME_H

#include "gui/Form.h"
#include "gui/Label.h"
#include "gui/Image.h"
#include "gui/Memo.h"
#include "gui/ShowMap.h"
#include "gui/ChildForm.h"
#include "gui/Edit.h"
#include "gui/BouttonText.h"
#include "Timer.h"

struct SDL_Thread;

class EChannel;
class ECPlayer;

/********************************************************************************************
 *                               TInGameForm                                                *
 ********************************************************************************************/

class TBarreActIcons : public TComponent
{
public:
	TBarreActIcons(int _x, int _y)
		: TComponent(_x,_y)
	{}
	~TBarreActIcons();

	void Init() {}

	void Draw(int souris_x, int souris_y);
	bool Clic (int mouse_x, int mouse_y);

	void SetList(std::vector<ECEntity*> list);
	void Clear();

	virtual void SetXY(int _x, int _y);

private:
	std::vector<TImage*> icons;
};

class TBarreAct : public TChildForm
{
public:
	TBarreAct(ECPlayer*);
	~TBarreAct();

/* Composants */
public:

	TImage*         Icon;
	TLabel*         Name;
	TLabel*         Nb;
	TLabel*         OwnerLabel;
	TLabel*         Owner;
	TButtonText*    MoveButton;
	TButtonText*    AttaqButton;
	TButtonText*    UpButton;
	TBarreActIcons* Icons;

/* Attributs */
public:

	ECEntity* Entity() const { return entity; }
	void SetEntity(ECEntity* e);

	ECase* Case() const { return acase; }
	void SetCase(ECase* c);

/* Methodes */
public:
	void Update() { if(entity) SetEntity(entity); }

	void Init();

	void UnSelect();
	bool Select() const { return select; }

	static void CreateUnit(TObject* o, void* e);

private:
	EChannel* chan;
	ECPlayer* me;
	ECEntity* entity;
	ECase* acase;
	static void vSetEntity(void*);
	static void vSetCase(void*);

	template<typename T>
	void ShowIcons(T e)
	{
		assert(Icons);
		if(e)
		{
			std::vector<ECEntity*> elist = EntityList.CanCreatedBy(e);
			Icons->SetList(elist);
		}
		else
			Icons->Clear();
	}

	bool select;
};

class TBarreLat : public TChildForm
{
public:
	TBarreLat(ECPlayer*);
	~TBarreLat();

	void Init();

/* Composants */
public:
	TImage*      Radar;
	TButtonText* PretButton;
	TButtonText* QuitButton;
	TButtonText* SchemaButton;
	TLabel*      Date;
	TLabel*      Money;
	TLabel*      TurnMoney;

/* Evenements */
public:
	static void RadarClick(TObject*, int, int);

protected:
	EChannel* chan;
	ECPlayer* player;
};

/** This is a form who show realy the map ! */
class TInGameForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TInGameForm(SDL_Surface*, ECPlayer*);
	~TInGameForm();

/* Composants */
public:

	TMap*      Map;
	TBarreLat* BarreLat;
	TBarreAct* BarreAct;

	TMemo*     Chat;
	TEdit*     SendMessage;

	SDL_Thread* Thread;

/* Methodes */
public:

	void ShowBarreLat(bool show = true);
	void ShowBarreAct(bool show = true);

	#define I_INFO     0x001
	#define I_WARNING  0x002
	#define I_ECHO     0x004
	#define I_SHIT     0x008
	#define I_CHAT     0x010
	#define I_GREAT    0x020
	void AddInfo(int flags, std::string line, ECPlayer* = 0);

/* Attributs */
public:
	Timer* GetTimer() { return &timer; }

/* Evenements */
public:

private:
	Timer timer;
	ECPlayer* player;
};

/********************************************************************************************
 *                               TLoadingForm                                               *
 ********************************************************************************************/
/** This is a form who shows a loading screen when the game is loading informations. */
class TLoadingForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TLoadingForm(SDL_Surface*, EChannel*);
	~TLoadingForm();

/* Composants */
public:

	TList*       Players;

	TLabel*      Title;
	TLabel*      Loading;

	TLabel*      MapTitle;
	TImage*      Preview;
	TLabel*      Date;
	TMemo*       MapInformations;

/* Evenements */
public:

};

/********************************************************************************************
 *                               TLoadPlayerLine                                            *
 ********************************************************************************************/
/** This is a line to show a player in TLoadingForm form */
class TLoadPlayerLine : public TComponent
{
/* Constructeur/Destructeur */
public:

	TLoadPlayerLine(ECPlayer *pl);
	virtual ~TLoadPlayerLine();

/* Methodes */
public:

	void Init();
	void Draw(int souris_x, int souris_y);              /**< Draw */

/* Composants */
public:

	TLabel* label;

/* Attributs */
public:
	ECPlayer* Player() { return pl; }

/* Variables priv�es */
private:

	/* Privatisation de constructeurs � rendre inaccessibles */
	TLoadPlayerLine();
	TLoadPlayerLine(int _x, int _y);
	TLoadPlayerLine(int _x, int _y, uint _w, uint _h);

	ECPlayer *pl;
};

#endif /* EC_INGAME_H */
