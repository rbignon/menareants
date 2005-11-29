/* src/JoinGame.cpp - Functions to list game and join/create a game.
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

#include "JoinGame.h"
#include "Main.h"
#include "Resources.h"
#include "Sockets.h"
#include "gui/ListBox.h"
#include "gui/BouttonText.h"
#include "gui/Menu.h"
#include "gui/Memo.h"
#include "gui/Edit.h"
#include "tools/Font.h"
#include "Outils.h"

ListBox *GameListBox = NULL;
Memo *MessageList = NULL;
bool EOL = false, JOINED = false;

/* <nick> MSG <message> */
int MSGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(MessageList) MessageList->AddItem("<" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? red_color : black_color);
	return 0;
}

/* LSP <nom> <nbjoueur> [nbmax] */
int LSPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(parv[3] == "0")
		GameListBox->AddItem(false, parv[1] + "   " + parv[2], parv[1], black_color, true);
	else
		GameListBox->AddItem(false, parv[1] + "   " + parv[2] + "/" + parv[3], parv[1],
		                 (parv.size() <= 3 || parv[2] != parv[3]) ? black_color : red_color,
		                 (parv.size() <= 3 || parv[2] != parv[3]) ? true : false);
	return 0;
}

/* EOL */
int EOLCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	EOL = true;
	return 0;
}

/* SETS [?] */
int SETSCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	return 0;
}

/* PLS [[@]nick] [[[@]nick] ...] */
int PLSCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!me->Player()) return 0; /* TODO Desynch */

	for(unsigned int i=1;i<parv.size();i++)
	{
		const char *nick = parv[i].c_str();
		bool owner = false;

		if(*nick == '@')
			owner = true, *nick++;

		if(!strcasecmp(nick, me->GetNick().c_str()))
		{ /* On a déjà créé notre player */
			if(owner) me->Player()->SetOwner();
		}
		else
			new ECPlayer(nick, me->Player()->Channel(), owner, false);
	}
	JOINED = true;
	return 0;
}

/* <nick> JOI <chan> */
int JOICommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(players.size())
	{ /* Le joueur est reconnu (!?) */
		/* TODO: Desynch */
	}
	else
	{ /* Le joueur est inconnu */
		if(parv[0] == me->GetNick())
		{ /* C'est moi qui join */
			EChannel *c = new EChannel(parv[1]);
			me->SetPlayer(new ECPlayer(parv[0].c_str(), c, false, true));
		}
		else
		{ /* C'est un user qui rejoin le chan */
			if(!me->Player()) return 0; /* TODO: Desynch */

			new ECPlayer(parv[0].c_str(), me->Player()->Channel(), false, false);
		}
	}
	return 0;
}

/* <nick> LEA */
int LEACommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	/* Note: Dans le protocole il ne devrait y avoir qu'un seul départ
	 *       à la fois mais bon par respect de ce qui est *possible*
	 *       (on ne sait jamais, après tout, on pourrait imager dans
	 *        un avenir des "services" qui pourraient "hacker" (plusieurs
	 *        départs ?) ou alors pour une eventuelle IA du serveur)
	 */
	for(unsigned int i=0; i<players.size(); i++)
	{
		if(players[i]->IsMe())
		{
			delete me->Player()->Channel();
			me->ClrPlayer();
			return 0;
		}
		else
			me->Player()->Channel()->RemovePlayer(players[i], true);
	}
	return 0;
}

void EuroConqApp::GameInfos(bool create)
{
	if(!client) return;
	if(create)
	{
		std::string name = Menu::EnterString("Nom", "", false);
		if(name.empty()) return;

		JOINED = false;
		client->sendrpl(client->rpl(EC_Client::JOIN), name.c_str());
		WAIT_EVENT(JOINED, i);
		if(!JOINED) return;
	}
	SDL_Event event;
	bool eob = false;
	EChannel *chan = client->Player()->Channel();

	/* Déclaration membres fixes */
	MessageList = new Memo(75,325,300,200,30);
	MessageList->Init();
	Edit SendMessage(75,530,300, MAXBUFFER-20);
	SendMessage.Init();
	ButtonText RetourButton(600,450,100,49, "Retour");
	RetourButton.SetFont(&normal_font);

	do
	{
		int x=0, y=0;
		while( SDL_PollEvent( &event) )
		{
			switch(event.type)
			{
				case SDL_KEYUP:
					SendMessage.PressKey(event.key.keysym);
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							eob = true;
							break;
						case SDLK_RETURN:
							if(SendMessage.Focused())
							{
								client->sendrpl(client->rpl(EC_Client::MSG),
								                FormatStr(SendMessage.GetString().c_str()));
								MessageList->AddItem("<" + client->GetNick() + "> " +
								                     SendMessage.GetString(), black_color);
								SendMessage.ClearString();
							}
							break;
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					MessageList->Clic( event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
					SendMessage.Clic(event.button.x, event.button.y);
					if(RetourButton.Test(event.button.x, event.button.y))
						eob = true;
					break;
				default:
					break;
			}
		}
		SDL_BlitSurface(Resources::Titlescreen()->Img,NULL,sdlwindow,NULL);
		SDL_GetMouseState( &x, &y);

		unsigned int vert = 30;
		for(unsigned int i=0; i<chan->Players().size();i++, vert += 50)
		{
			ECPlayer *pl = ((ECPlayer*)chan->Players()[i]);

			big_font.WriteLeft(30, vert, "OK", pl->Ready ? black_color : gray_color);
			if(pl->IsOwner()) big_font.WriteLeft(70, vert, "*", red_color);
			big_font.WriteLeft(85, vert, pl->GetNick(), black_color);
		}
		//big_font.WriteCenter(400,180, "Liste des parties", black_color);
		MessageList->Display(x,y);
		SendMessage.Display();

		RetourButton.Draw(x, y);
		SDL_Flip(sdlwindow);
	} while(!eob && client->IsConnected() && client->Player());

	delete MessageList;
	MessageList = NULL;

	client->sendrpl(client->rpl(EC_Client::LEAVE));
	return;
}

void EuroConqApp::ListGames()
{
	if(!client) return;

	SDL_Event event;

	/* Déclaration membres fixes */
	ButtonText JoinButton(500,200,100,49, "Joindre");
	JoinButton.SetFont(&normal_font);
	ButtonText RefreshButton(500,250,100,49, "Actualiser");
	RefreshButton.SetFont(&normal_font);
	ButtonText CreerButton(500,300,100,49, "Creer");
	CreerButton.SetFont(&normal_font);
	ButtonText RetourButton(500,350,100,49, "Retour");
	RetourButton.SetFont(&normal_font);

	GameListBox = new ListBox(300,200,200,300);
	GameListBox->Init();

	bool eob = false, refresh = true;
	int i;
	do
	{
		if(refresh)
		{
			GameListBox->ClearItems();
			EOL = false;
			client->sendrpl(client->rpl(EC_Client::LISTGAME));
			WAIT_EVENT(EOL, j);
			if(!EOL)
			{
				delete GameListBox;
				return; /* On a attendu pour rien */
			}
			refresh = false;
			JoinButton.SetEnabled(false);
			i=0;
		}
		int x=0, y=0;
		while( SDL_PollEvent( &event) )
		{
			switch(event.type)
			{
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							eob = true;
							break;
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					GameListBox->Clic( event.button.x, event.button.y);
					if(JoinButton.Enabled() && JoinButton.Test(event.button.x, event.button.y))
					{
						JOINED = false;
						client->sendrpl(client->rpl(EC_Client::JOIN),
							GameListBox->ReadValue(GameListBox->GetSelectedItem()).c_str());
						WAIT_EVENT(JOINED, j);
						if(JOINED)
							GameInfos(false);
					}
					else if(RefreshButton.Test(event.button.x, event.button.y))
						refresh = true;
					else if(CreerButton.Test(event.button.x, event.button.y))
						GameInfos(true);
					else if(RetourButton.Test(event.button.x, event.button.y))
						eob = true;
					break;
				default:
					break;
			}
		}
		SDL_BlitSurface(Resources::Titlescreen()->Img,NULL,sdlwindow,NULL);
		SDL_GetMouseState( &x, &y);
		big_font.WriteCenter(400,180, "Liste des parties", black_color);
		GameListBox->Display(x,y);
		if(GameListBox->GetSelectedItem() >= 0 &&
		   GameListBox->Enabled(GameListBox->GetSelectedItem()))
			JoinButton.SetEnabled(true);
		JoinButton.Draw(x, y);
		RefreshButton.Draw(x, y);
		CreerButton.Draw(x, y);
		RetourButton.Draw(x, y);
		SDL_Flip(sdlwindow);
		if(++i >= 100) refresh = true; /* ~= une minute ? */
	} while(!eob && client->IsConnected());

	delete GameListBox;
	GameListBox=NULL;
	return;
}
