/* src/Login.h - Header of Login.cpp
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
 * $Id$
 */

#ifndef EC_LOGIN_H
#define EC_LOGIN_H

#include "gui/Form.h"
#include "gui/BouttonText.h"
#include "gui/ListBox.h"
#include "gui/Memo.h"
#include "gui/Label.h"
#include "Timer.h"

class EC_Client;

class TGlobalScoresForm : public TForm
{
public:

	TGlobalScoresForm(ECImage*);

	static void Scores(TObject*, void*);

public:

	TLabel*      Title;
	TLabel*      BestPlayer;
	TLabel*      BestIncomes;
	TLabel*      BestKills;
	TLabel*      Headers;
	TListBox*    ListBox;
	TButtonText* RetourButton;

	void OnClic(const Point2i& position, int button, bool& stop);

	int best_incomes, best_kills;
};

/** This is class based on TForm showed when I am connected */
class TConnectedForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TConnectedForm(ECImage*);

/* Composants */
public:

	TMemo*       Motd;
	TButtonText* CreerButton;
	TButtonText* JoinButton;
	TButtonText* RefreshButton;
	TButtonText* DisconnectButton;
	TButtonText* RehashButton;
	TButtonText* KillButton;
	TLabel*      Welcome;
	TLabel*      UserStats;
	TLabel*      ChanStats;
	TLabel*      ServerStats;
	TLabel*      Uptime;
	TLabel*      ListLabel;
	TLabel*      MOTDLabel;
	TListBox*    GList;

/* Variables publiques */
public:

	std::string Rejoin;
	bool refresh;

	void SetClient(EC_Client* cl) { client = cl; }

private:
	EC_Client* client;
	Timer timer;
	void AfterDraw();
	void OnClic(const Point2i& position, int button, bool& stop);
};

/** This is class based on TForm show a server list received from a meta-server */
class TListServerForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TListServerForm(ECImage*);

/* Composants */
public:

	TListBox*    ServerList;
	TButtonText* RetourButton;
	TButtonText* RefreshButton;
	TButtonText* ConnectButton;
	TButtonText* ConnectToButton;
	TButtonText* RegisterButton;
	TButtonText* StatsButton;
	TButtonText* AccountButton;
	TButton*     MissionButton;
	TButton*     EscarmoucheButton;
	TLabel*      Label1;
	TLabel*      Label2;
	TLabel*      UserStats;
	TLabel*      ChanStats;
	TLabel*      Welcome;

	uint nb_chans, nb_wchans, nb_users, nb_tchans, nb_tusers, nb_tregs;

	std::string Rejoin;

	bool login;

private:
	Timer timer;
	void AfterDraw();
	void OnClic(const Point2i& position, int button, bool& stop);
};
#endif
