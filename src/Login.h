/* src/Login.h - Header of Login.cpp
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
 * $Id$
 */

#ifndef EC_LOGIN_H
#define EC_LOGIN_H

#include "gui/Form.h"
#include "gui/BouttonText.h"
#include "gui/Memo.h"
#include "gui/Label.h"

/** This is class based on TForm showed when I am connected */
class TConnectedForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TConnectedForm(SDL_Surface*);
	~TConnectedForm();

/* Composants */
public:

	TMemo*       Motd;
	TButtonText* CreateButton;
	TButtonText* ListButton;
	TButtonText* DisconnectButton;
	TLabel*      Welcome;
	TLabel*      UserStats;
	TLabel*      ChanStats;
	TLabel*      Uptime;

/* Evenements */
public:

};
#endif
