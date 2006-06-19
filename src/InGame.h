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

class TBarreActIcons : public TChildForm
{
public:
	TBarreActIcons(int _x, int _y)
		: TChildForm(_x,_y, 0, 0)
	{}

	void Init() {}

	void SetList(std::vector<ECEntity*> list);

private:
	std::vector<TImage*> icons;
};

class TBarreLatIcons : public TChildForm
{
public:
	TBarreLatIcons(int _x, int _y)
		: TChildForm(_x,_y, 0, 0)
	{}

	void Init() {}

	void SetList(std::vector<ECEntity*> list);

private:
	std::vector<TImage*> icons;
	static void SelectUnit(TObject* o, void* e);
};

class TBarreAct : public TChildForm
{
public:
	TBarreAct(ECPlayer*);

/* Composants */
public:

	TImage*         Icon;
	TLabel*         Name;
	TLabel*         Nb;
	TLabel*         Owner;
	TLabel*         Infos;
	TButtonText*    DeployButton;
	TButtonText*    AttaqButton;
	TButtonText*    UpButton;
	TBarreActIcons* Icons;

/* Attributs */
public:

	ECEntity* Entity() const { return entity; }
	void SetEntity(ECEntity* e);

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
	static void vSetEntity(void*);

	template<typename T>
	void ShowIcons(T e, ECBPlayer* pl)
	{
		assert(Icons);
		if(e)
		{
			std::vector<ECEntity*> elist = EntityList.CanCreatedBy(e, pl);
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

	void Init();

/* Composants */
public:
	TImage*         Radar;
	TButtonText*    PretButton;
	TButtonText*    QuitButton;
	TButtonText*    SchemaButton;
	TButtonText*    OptionsButton;
	TLabel*         Date;
	TLabel*         Money;
	TLabel*         TurnMoney;
	TMemo*          UnitsInfos;
	TBarreLatIcons* Icons;
	TImage*         ScreenPos;

	EChannel* Channel() const { return chan; }

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

	ECPlayer* Player() const { return player; }

/* Evenements */
public:

private:
	Timer timer;
	ECPlayer* player;
};

/********************************************************************************************
 *                               TOptionsForm                                               *
 ********************************************************************************************/
/** There is options. */
class TOptionsForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TOptionsForm(SDL_Surface*, ECPlayer*, EChannel*);
	~TOptionsForm();

/* Composants */
public:

	TList*       Players;

	TButtonText* OkButton;

/* Evenements */
public:

};

/********************************************************************************************
 *                               TOptionsPlayerLine                                         *
 ********************************************************************************************/
/** This is a line to show a player in TOptionsForm form */
class TOptionsPlayerLine : public TComponent
{
/* Constructeur/Destructeur */
public:

	TOptionsPlayerLine(ECPlayer* me, ECPlayer *pl);
	virtual ~TOptionsPlayerLine();

/* Methodes */
public:

	void Init();
	void Draw(int souris_x, int souris_y);              /**< Draw */

	bool AllieZone(int _x, int _y);

/* Composants */
public:

	TLabel* label;
	TLabel* allie;
	TLabel* recipr;

/* Attributs */
public:
	ECPlayer* Player() { return pl; }

/* Variables privées */
private:

	/* Privatisation de constructeurs à rendre inaccessibles */
	TOptionsPlayerLine();
	TOptionsPlayerLine(int _x, int _y);
	TOptionsPlayerLine(int _x, int _y, uint _w, uint _h);

	ECPlayer *pl;
	ECPlayer *me;
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

/* Variables privées */
private:

	/* Privatisation de constructeurs à rendre inaccessibles */
	TLoadPlayerLine();
	TLoadPlayerLine(int _x, int _y);
	TLoadPlayerLine(int _x, int _y, uint _w, uint _h);

	ECPlayer *pl;
};

/********************************************************************************************
 *                               TScoresForm                                                *
 ********************************************************************************************/
/** This is a form who shows the channel's scores. */
class TScoresForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TScoresForm(SDL_Surface*, EChannel*);
	~TScoresForm();

/* Composants */
public:

	TLabel*      Title;
	TLabel*      Date;

	TButtonText* RetourButton;

	TList*       Players;

/* Evenements */
public:

	static void WantLeave(TObject*, void*);
};

/********************************************************************************************
 *                               TScoresPlayerLine                                          *
 ********************************************************************************************/
/** This is a line to show a player in TScoresForm form */
class TScoresPlayerLine : public TComponent
{
/* Constructeur/Destructeur */
public:

	TScoresPlayerLine(std::string nick, SDL_Color, std::string killed, std::string shooted, std::string created,
	                  std::string score);
	virtual ~TScoresPlayerLine();

/* Methodes */
public:

	void Init();
	void Draw(int souris_x, int souris_y);              /**< Draw */

/* Composants */
public:

	TLabel* Nick;
	TLabel* Killed;
	TLabel* Shooted;
	TLabel* Created;
	TLabel* Score;

/* Variables privées */
private:
	std::string nick;
	SDL_Color color;
	std::string killed, shooted, created, score;
};

#endif /* EC_INGAME_H */
