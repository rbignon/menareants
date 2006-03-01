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
#include "Channels.h"

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
	void Draw(uint souris_x, uint souris_y);              /**< Draw */

/* Composants */
public:

/* Attributs */
public:
	ECPlayer* Player() { return pl; }

/* Variables priv�es */
private:

	/* Privatisation de constructeurs � rendre inaccessibles */
	TLoadPlayerLine();
	TLoadPlayerLine(uint _x, uint _y);
	TLoadPlayerLine(uint _x, uint _y, uint _w, uint _h);

	ECPlayer *pl;
};

#endif /* EC_INGAME_H */