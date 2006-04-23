/* src/JoinGame.h - Header of JoinGame.cpp
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

/********************************************************************************************
 *                               TGameInfosForm                                              *
 ********************************************************************************************/
/** This is a form based on TForm showed when an user joined or created a channel before start game. */
class TGameInfosForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TGameInfosForm(SDL_Surface*);
	~TGameInfosForm();

/* Composants */
public:

	TMemo*       Chat;
	TEdit*       SendMessage;
	TButtonText* RetourButton;
	TButtonText* PretButton;
	TList*       Players;
	TLabel*      Title;
	TListBox*    MapList;
	TLabel*      MapTitle;
	TImage*      Preview;

	TMemo*       Hints;

	TSpinEdit*   MyPosition;
	TColorEdit*  MyColor;
	TComboBox*   MyNation;

/* Variables publiques */
public:
	bool RecvMapList;
	std::vector<std::string> RecvMap;

/* Evenements */
public:

/* Fonctions */
public:

	/* Calcul la taille du TMemo */
	void RecalcMemo();

};

/********************************************************************************************
 *                               TListGameForm                                              *
 ********************************************************************************************/
/** This is a form based on TForm showed when client want to list channels. */
class TListGameForm : public TForm
{
/* Constructeur/Deconstructeur */
public:

	TListGameForm(SDL_Surface*);
	~TListGameForm();

/* Composants */
public:

	TButtonText  *JoinButton;
	TButtonText  *RefreshButton;
	TButtonText  *CreerButton;
	TButtonText  *RetourButton;
	TListBox     *GList;
	TLabel       *Title;

/* Evenements */
public:

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
	void Draw(int souris_x, int souris_y);                /**< Draw */
	virtual void SetXY (int _x, int _y);                  /**< Set \a x and \a y positions */

	bool OwnZone(int _x, int _y);

	virtual bool Test (int souris_x, int souris_y) const;

/* Composants */
public:

	TSpinEdit   *position;
	TColorEdit  *couleur;
	TComboBox   *nation;

/* Attributs */
public:
	ECPlayer* Player() { return pl; }

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
	void Draw(int souris_x, int souris_y);                /**< Draw */
	virtual void SetXY (int _x, int _y);                  /**< Set \a x and \a y positions */

/* Composants */
public:
	TLabel *label;
};

#endif /* EC_JOINGAME_H */
