/* src/JoinGame.h - Header of JoinGame.cpp
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

#ifndef EC_JOINGAME_H
#define EC_JOINGAME_H

#include "Channels.h"
#include "gui/Form.h"
#include "gui/BouttonText.h"
#include "gui/ListBox.h"
#include "gui/Label.h"
#include "gui/Edit.h"
#include "gui/Memo.h"
#include "gui/SpinEdit.h"
#include "gui/ColorEdit.h"
#include "gui/Image.h"
#include "gui/ComboBox.h"
#include "gui/CheckBox.h"

class EC_Client;

/********************************************************************************************
 *                               TGameInfosForm                                             *
 ********************************************************************************************/
/** This is a form based on TForm showed when an user joined or created a channel before start game. */
class TGameInfosForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TGameInfosForm(ECImage*, EC_Client*, bool mission = false);

/* Composants */
public:

	TMemo*       Chat;
	TEdit*       SendMessage;
	TButtonText* RetourButton;
	TButtonText* PretButton;
	TButtonText* CreateIAButton;
	TList*       Players;
	TLabel*      Title;
	TListBox*    MapList;
	TCheckBox*   SpeedGame;
	TSpinEdit*   BeginMoney;
	TSpinEdit*   TurnTime;
	TLabel*      MapTitle;
	TImage*      Preview;

	TSpinEdit*   MyPosition;
	TColorEdit*  MyColor;
	TComboBox*   MyNation;

/* Variables publiques */
public:
	bool RecvMapList, mission;
	static std::vector<std::string> RecvMap;

/* Fonctions */
public:

	/* Calcul la taille du TMemo */
	void RecalcMemo();

	void ChangeStatus(bool);

/* Evenements */
private:

	void OnClic(const Point2i&, int, bool&);
	void OnKeyUp(SDL_keysym key);
	void AfterDraw();
	static void MapListChange(TListBox* MapList);
	static void SpinEditChange(TSpinEdit* SpinEdit);

/* Variables privées */
private:

	Timer listmapclick;
	EC_Client* client;
};

/********************************************************************************************
 *                               TPlayerLine                                                *
 ********************************************************************************************/
/** This is a line to show a player status in TGameInfos form */
class TPlayerLine : public TComponent
{
/* Constructeur/Destructeur */
public:

	TPlayerLine(ECPlayer *pl);

	~TPlayerLine();

/* Methodes */
public:

	void Init();                                          /**< Initialisation */
	void Draw(const Point2i&);                /**< Draw */
	virtual void SetXY (int _x, int _y);                  /**< Set \a x and \a y positions */

	bool OwnZone(const Point2i&, int button);

	virtual bool Test (const Point2i&, int button) const;

/* Composants */
public:

	TSpinEdit   *position;
	TColorEdit  *couleur;
	TComboBox   *nation;
	TLabel*     Ready;
	TLabel*     Status;
	TLabel*     Nick;

/* Attributs */
public:
	ECPlayer* Player() { return pl; }

	virtual bool IsHint(const Point2i& pos) const { return HaveHint(); }

/* Variables privées */
private:

	/* Privatisation de constructeurs à rendre inaccessibles */
	TPlayerLine();
	TPlayerLine(int _x, int _y);
	TPlayerLine(int _x, int _y, uint _w, uint _h);

	ECPlayer *pl;
};

/********************************************************************************************
 *                               TPlayerLineHeader                                          *
 ********************************************************************************************/
/** This is a line to show a the header of playerlist in TGameInfos form */
class TPlayerLineHeader : public TComponent
{
/* Constructeur/Destructeur */
public:

	TPlayerLineHeader();
	~TPlayerLineHeader();

/* Methodes */
public:

	void Init();                                          /**< No initialisation */
	void Draw(const Point2i&);                /**< Draw */
	virtual void SetXY (int _x, int _y);                  /**< Set \a x and \a y positions */

/* Composants */
public:
	TLabel *label;
};

#endif /* EC_JOINGAME_H */
