/* src/JoinGame.h - Header of JoinGame.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#include "gui/Form.h"
#include "gui/BouttonText.h"
#include "gui/ListBox.h"
#include "gui/Label.h"
#include "gui/Edit.h"
#include "gui/Memo.h"

class TGameInfosForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TGameInfosForm();
	~TGameInfosForm();

/* Composants */
public:

	TMemo*       Chat;
	TEdit*       SendMessage;
	TButtonText* RetourButton;

/* Evenements */
public:

};

class TListGameForm : public TForm
{
/* Constructeur/Deconstructeur */
public:

	TListGameForm();
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

#endif /* EC_JOINGAME_H */
