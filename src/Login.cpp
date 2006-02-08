/* src/Login.cpp - Login commands
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
 * $Id$
 */

#include "Commands.h"
#include "Sockets.h"
#include "Debug.h"
#include "tools/Font.h"
#include "gui/MessageBox.h"
#include "Main.h"
#include "Login.h"
#include "Resources.h"

TConnectedForm  *ConnectedForm = NULL;
static bool RC_MOTD = false;

/** We are connected to server.
 *
 * Syntax: HEL prog version
 */
int HELCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->sendrpl(me->rpl(EC_Client::IAM), me->lapp->getconf()->nick.c_str());
	return 0;
}

/** Server acknowledges my nickname.
 *
 * Syntax: AIM nick
 */
int AIMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->set_nick(parv[1]);
	me->SetConnected();
	return 0;
}

/** Received a PING from server.
 *
 * Syntax: PIG
 */
int PIGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->sendrpl(me->rpl(EC_Client::PONG));

	return 0;
}

/** My nickname is already used by another user.
 *
 * Syntax: USED
 */
int USEDCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->SetCantConnect("Le pseudo " + me->lapp->getconf()->nick + " est pris");
	return 0;
}

/** My game isn't compatible with this server.
 *
 * Syntax: MAJ <0/+/->
 */
int MAJCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	switch(parv[1][0])
	{
		case '0':
			me->SetCantConnect("Ce jeu n'est pas compatible avec ce serveur !?");
			break;
		case '+':
			me->SetCantConnect("Vous utilisez une version du jeu plus récente que celle "
			                   "supportée par le serveur");
			break;
		case '-':
			me->SetCantConnect("Vous devez mettre à jour Europa Conquest pour jouer "
			                   "sur ce serveur");
			break;
		default:
			me->SetCantConnect("Impossible de se connecter");
			vDebug(W_DESYNCH|W_SEND, "Reception d'un MAJ bizarre !",
		                                VName(parv[1]));
			break;
	}
	return 0;
}

/** I receive message of the day of server.
 *
 * Syntax: MOTD [ligne]
 */
int MOTDCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!ConnectedForm) return 0;

	if(!RC_MOTD)
	{
		RC_MOTD = true;
		ConnectedForm->Motd->ClearItems();
	}

	ConnectedForm->Motd->AddItem(parv.size() > 1 ? parv[1] : "", black_color);
	return 0;
}

/** I received all message of the day from server.
 *
 * Syntax: EOM
 */
int EOMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	RC_MOTD = false;
	return 0;
}

void EuroConqApp::request_game()
{
	if(client)
	{
		Debug(W_SEND|W_ERR, "Appel de EuroConqApp::request_game() en étant déjà connecté !");
		return;
	}

	try
	{
		ConnectedForm = new TConnectedForm;

		SDL_mutex *Mutex = SDL_CreateMutex();
		Thread = SDL_CreateThread(EC_Client::read_sock, Mutex);

		WAIT_EVENT_T((client && client->IsConnected()), i, 15);

		if(!client || !client->IsConnected())
		{
			std::string msg;
			if(!client)
				msg = "Connexion impossible";
			else
				msg = "Connexion impossible :\n" + client->CantConnect();
			TMessageBox mb(100,200, msg.c_str(), BT_OK, NULL);
			mb.SetBackGround(Resources::Titlescreen());
			mb.Show();
			if(client)
			{
				delete client;
				client = 0;
			}
			delete ConnectedForm;
			ConnectedForm = 0;
			return;
		}

		ConnectedForm->Welcome->SetCaption("Vous etes bien connecte en temps que " +
		                                               client->GetNick());

		bool eob = false;
		SDL_Event event;
		do
		{
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
					case SDL_MOUSEBUTTONUP:
						ConnectedForm->Motd->Clic( event.button.x, event.button.y);
						break;
					case SDL_MOUSEBUTTONDOWN:
						if(ConnectedForm->DisconnectButton->Test(event.button.x, event.button.y))
						{
							client->SetWantDisconnect();
							client->sendrpl(client->rpl(EC_Client::BYE));
							eob = true;
						}
						if(ConnectedForm->ListButton->Test(event.button.x, event.button.y))
							ListGames();
						if(ConnectedForm->CreateButton->Test(event.button.x, event.button.y))
							GameInfos(NULL);
						break;
					default:
						break;
				}
			}
			SDL_GetMouseState( &x, &y);

			ConnectedForm->Update(x, y, false);

			SDL_Flip(sdlwindow);

			if(!client || !client->IsConnected())
			{
				eob = true;

				TMessageBox mb(100,200, "Vous avez ete deconnecte.", BT_OK, NULL);
				mb.SetBackGround(Resources::Titlescreen());
				mb.Show();
			}
		} while(!eob);
	}
	catch(const TECExcept &e)
	{
		Debug(W_ERR, e.Message);
		SDL_KillThread(Thread); /* En cas d'erreur, clore arbitrairement le thread */
	}
	if(client)
	{
		delete client;
		client = NULL;
	}
	SDL_WaitThread(Thread, 0);

	delete ConnectedForm;
	ConnectedForm = NULL;

	return;
}

/********************************************************************************************
 *                               TConnectedForm                                             *
 ********************************************************************************************/

TConnectedForm::TConnectedForm()
	: TForm()
{
	Welcome = AddComponent(new TLabel(200,50,"Vous etes bien connecte", black_color, &big_font));

	Motd = AddComponent(new TMemo(75,100,500,400, 0));

	CreateButton = AddComponent(new TButtonText(600,100, 100,49, "Créer une partie"));
	ListButton = AddComponent(new TButtonText(600,150,100,49, "Lister les parties"));
	DisconnectButton = AddComponent(new TButtonText(600,200,100,49, "Se déconnecter"));

	SetBackground(Resources::Menuscreen());
}

TConnectedForm::~TConnectedForm()
{
	delete Welcome;
	delete Motd;
	delete CreateButton;
	delete ListButton;
	delete DisconnectButton;
}
