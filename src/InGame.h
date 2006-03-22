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

class EChannel;
class ECPlayer;

/********************************************************************************************
 *                               TInGameForm                                                *
 ********************************************************************************************/
class TBarreLat : public TChildForm
{
public:
	TBarreLat(EChannel*);
	~TBarreLat();

/* Composants */
public:
	TImage*      Radar;
	TButtonText* PretButton;

protected:
	EChannel* chan;
};

/** This is a form who show realy the map ! */
class TInGameForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TInGameForm(EChannel*);
	~TInGameForm();

/* Composants */
public:

	TMap*      Map;
	TBarreLat* BarreLat;

	TMemo*     Chat;
	TEdit*     SendMessage;

/* Methodes */
public:

	void ShowBarreLat(bool show = true);

/* Evenements */
public:

};

/********************************************************************************************
 *                               TLoadingForm                                               *
 ********************************************************************************************/
/** This is a form who shows a loading screen when the game is loading informations. */
class TLoadingForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TLoadingForm(EChannel*);
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

/* Methodes */
public:

	void Init() {}
	void Draw(int souris_x, int souris_y);              /**< Draw */

/* Composants */
public:

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

#endif /* EC_INGAME_H */
