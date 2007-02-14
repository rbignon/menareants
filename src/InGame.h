/* src/InGame.h - Header of InGame.cpp
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

#ifndef EC_INGAME_H
#define EC_INGAME_H

#include "gui/Form.h"
#include "gui/Label.h"
#include "gui/Image.h"
#include "gui/Memo.h"
#include "gui/ShowMap.h"
#include "gui/ChildForm.h"
#include "gui/Edit.h"
#include "gui/Fps.h"
#include "gui/BouttonText.h"
#include "gui/ProgressBar.h"
#include "Timer.h"

struct SDL_Thread;

class EChannel;
class ECPlayer;
class TCursor;

/********************************************************************************************
 *                               TInGameForm                                                *
 ********************************************************************************************/

class TBarreActIcons : public TChildForm
{
public:
	TBarreActIcons(int _x, int _y)
		: TChildForm(_x,_y, 0, 0), Next(0), Last(0), max_height(200), first(0)
	{}

	void SetMaxHeight(uint i) { max_height = i; }

	void Init();

	TButton* Next;
	TButton* Last;

	void SetList(std::vector<ECEntity*> list, bool click = true);

	virtual bool Clic(const Point2i&, int button);

private:
	std::vector<TImage*> icons;
	uint max_height, first;
};

class TBarreLatIcons : public TChildForm
{
public:
	TBarreLatIcons(int _x, int _y)
		: TChildForm(_x,_y, 0, 0), Next(0), Last(0), max_height(100), first(0)
	{}

	void SetMaxHeight(uint i) { max_height = i; }

	//virtual void SetXY (int _x, int _y) { TChildForm::SetXY(_x,_y); Init(); }

	void Init();

	TButton* Next;
	TButton* Last;

	void SetList(std::vector<ECEntity*> list, TOnClickFunction func);

	virtual bool Clic(const Point2i&, int button);

private:
	std::vector<TImage*> icons;
	uint max_height, first;
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
	TLabel*         SpecialInfo;
	TButtonText*    ExtractButton;
	TButtonText*    DeployButton;
	TButtonText*    UpButton;
	TButtonText*    UpgradeButton;
	TButtonText*    HelpButton;
	TButtonText*    GiveButton;
	TBarreActIcons* Icons;

	TImage*         ChildIcon;
	TLabel*         ChildNb;

	TMemo*          HelpInfos;
	TLabel*         HelpAttaqs;

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

	void ShowInfos();

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

	bool select, infos;
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
	TButtonText*    BaliseButton;
	TButtonText*    IdleFindButton;
	TProgressBar*   ProgressBar;
	TLabel*         Date;
	TLabel*         Money;
	TLabel*         TurnMoney;
	TMemo*          UnitsInfos;
	TBarreLatIcons* Icons;
	TImage*         ScreenPos;

	EChannel* Channel() const { return chan; }

/* Evenements */
public:
	static void RadarClick(TObject*, const Point2i&);
	static void SelectUnit(TObject* o, void* e);

protected:
	EChannel* chan;
	ECPlayer* player;
};

/** This is a form who show realy the map ! */
class TInGameForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TInGameForm(ECImage*, EC_Client*);

	enum Wants
	{
		W_NONE,
		W_REMBP,
		W_ADDBP,
		W_INVEST,
		W_UNSELECT,
		W_CANTATTAQ,
		W_MATTAQ,
		W_SELECT,
		W_EXTRACT,
		W_ATTAQ,
		W_MOVE
	};

/* Composants */
public:

	TMap*      Map;
	TBarreLat* BarreLat;
	TBarreAct* BarreAct;

	TMemo*     Chat;
	TEdit*     SendMessage;
	TFPS*      FPS;

	TCursor*   Cursor;

	SDL_Thread* Thread;

/* Methodes */
public:

	void ShowBarreLat(bool show = true);
	void ShowBarreAct(bool show = true);

	void SetCursor();
	Wants GetWant(ECEntity* entity, int button_type);

	#define I_INFO     0x001
	#define I_WARNING  0x002
	#define I_ECHO     0x004
	#define I_SHIT     0x008
	#define I_CHAT     0x010
	#define I_GREAT    0x020
	void AddInfo(int flags, std::string line, ECPlayer* = 0);

	void FindIdling();

/* Attributs */
public:
	Timer* GetTimer() { return &timer; }
	Timer* GetElapsedTime() { return &elapsed_time; }

	ECPlayer* Player() const { return player; }

	std::string ShowWaitMessage;
	bool WantBalise;

/* Evenements */
private:

	void OnMouseMotion(const Point2i& mouse);
	void OnClic(const Point2i& mouse, int, bool&);
	void OnClicUp(const Point2i& mouse, int button);
	void OnKeyUp(SDL_keysym);
	void OnKeyDown(SDL_keysym);
	void AfterDraw();
	void BeforeDraw();

private:
	Timer timer;
	Timer elapsed_time;
	ECPlayer* player;
	EC_Client* client;
	EChannel* chan;
	Wants want;
};

/********************************************************************************************
 *                               TOptionsForm                                               *
 ********************************************************************************************/
/** There is options. */
class TOptionsForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TOptionsForm(ECImage*, EC_Client*, EChannel*);

/* Composants */
public:

	TLabel*      Title;
	TLabel*      Label1;

	TList*       Players;

	TButtonText* SaveButton;
	TButtonText* OkButton;

/* Evenements */
private:

	void OnClic(const Point2i&, int, bool&);
	void AfterDraw();

/* Variables privées */
private:
	EC_Client* client;
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
	void Draw(const Point2i&);              /**< Draw */

	bool AllieZone(const Point2i&, int button);

/* Composants */
public:

	TLabel* label;
	TLabel* allie;
	TLabel* recipr;
	TButtonText* GiveMoneyButton;

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

	TLoadingForm(ECImage*, EChannel*);

/* Composants */
public:

	TList*       Players;

	TLabel*      Title;
	TLabel*      Loading;

	TLabel*      MapTitle;
	TImage*      Preview;
	TLabel*      Date;
	TMemo*       MapInformations;

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
	void Draw(const Point2i&);              /**< Draw */

/* Composants */
public:

	TLabel* label;
	TLabel* ready;

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
/** Form to the +Q (PINGING) mode */
class TPingingForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TPingingForm(ECImage*, EC_Client* client, EChannel*);

/* Composants */
public:

	TLabel*       Title;
	TMemo*        Message;
	TButtonText*  LeaveButton;
	TList*        Players;

/* Attributs */
public:
	EChannel* Channel() const { return channel; }

/* Methodes */
public:
	void UpdateList();

/* Evenements */
private:

	void AfterDraw();
	void BeforeDraw();
	void OnClic(const Point2i& mouse, int button, bool& stop);

/* Variables privées */
private:
	EChannel* channel;
	EC_Client* client;
};

class TPingingPlayerLine : public TChildForm
{
public:

	TPingingPlayerLine(ECBPlayer* pl);

	TLabel*       Nick;
	TLabel*       NbVotes;
	TButtonText*  Voter;
	TProgressBar* Progress;

	Timer         timer;

	virtual void Init();

	ECBPlayer* Player() const { return player; }

private:
	ECBPlayer* player;
};

/********************************************************************************************
 *                               TScoresForm                                                *
 ********************************************************************************************/
/** This is a form who shows the channel's scores. */
class TScoresForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TScoresForm(ECImage*, EChannel*);

/* Composants */
public:

	TLabel*      Title;
	TLabel*      Date;
	TLabel*      InitDate;
	TLabel*      Duree;

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

	TScoresPlayerLine(std::string nick, Color, std::string killed, std::string shooted, std::string created,
	                  std::string score);
	virtual ~TScoresPlayerLine();

/* Methodes */
public:

	void Init();
	void Draw(const Point2i&);              /**< Draw */

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
	Color color;
	std::string killed, shooted, created, score;
};

#endif /* EC_INGAME_H */
