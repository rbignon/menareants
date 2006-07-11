/* src/Login.cpp - Login commands
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

#include "Commands.h"
#include "Sockets.h"
#include "Debug.h"
#include "tools/Font.h"
#include "tools/Video.h"
#include "gui/MessageBox.h"
#include "Login.h"
#include "Resources.h"
#include "Outils.h"
#include "Main.h"
#include "Config.h"

TConnectedForm  *ConnectedForm = NULL;
static bool RC_MOTD = false;

/** We are connected to server.
 *
 * Syntax: HEL prog version
 */
int HELCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->sendrpl(me->rpl(EC_Client::IAM), Config::GetInstance()->nick.c_str());
	return 0;
}

/** Server acknowledges my nickname.
 *
 * Syntax: AIM nick
 */
int AIMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->set_nick(parv[1]);
	me->UnsetLogging();
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
	me->SetCantConnect("Le pseudo " + Config::GetInstance()->nick + " est pris");
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
			me->SetCantConnect("Vous devez mettre à jour Men Are Ants pour jouer "
			                   "sur ce serveur.\n\nAllez sur " APP_SITE " pour plus d'informations.");
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

/** Statistics of server.
 *
 * Syntax: STAT nbactco nbco nbch chinwait chingame chtot uptime
 */
int STATCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(ConnectedForm)
	{
		ConnectedForm->Uptime->SetCaption("Le serveur a été lancé il y a " +
		                                  std::string(duration(time(NULL) - StrToTyp<time_t>(parv[7]))));
		ConnectedForm->UserStats->SetCaption("Il y a " + parv[1] + " personnes connectées, avec " + parv[2] +
		                                     " connexions totales et " + parv[6] + " parties jouées.");
		ConnectedForm->ChanStats->SetCaption("Il y a actuellement " + parv[3] + " partie(s), dont " + parv[5] +
		                                     " en cours de jeu et " + parv[4] + " en préparation");
	}
	return 0;
}

void MenAreAntsApp::request_game()
{
	if(EC_Client::GetInstance())
	{
		FDebug(W_SEND|W_ERR, "Appel en étant déjà connecté !");
		return;
	}

	try
	{
		ConnectedForm = new TConnectedForm(Video::GetInstance()->Window());

		//SDL_mutex* Mutex = SDL_CreateMutex();
		mutex = SDL_CreateMutex();
		Thread = SDL_CreateThread(EC_Client::read_sock, mutex);

		WAIT_EVENT_T((EC_Client::GetInstance() && (EC_Client::GetInstance()->IsConnected() ||
		             EC_Client::GetInstance()->Error())), i, 5);

		EC_Client* client = EC_Client::GetInstance();

		if(!client || !client->IsConnected())
		{
			std::string msg;
			if(client)
				client->SetWantDisconnect();

			if(!client)
				msg = "Connexion impossible";
			else
				msg = "Connexion impossible :\n\n" + client->CantConnect();
			TMessageBox mb(msg.c_str(), BT_OK, NULL);
			mb.SetBackGround(Resources::Titlescreen());
			mb.Show();
			if(client)
			{
				delete client;
				EC_Client::singleton = NULL;
			}
			delete ConnectedForm;
			ConnectedForm = 0;
			SDL_WaitThread(Thread, 0);
			if(mutex)
			{
				SDL_DestroyMutex(mutex);
				mutex = 0;
			}
			return;
		}

		ConnectedForm->SetMutex(mutex);
		ConnectedForm->Welcome->SetCaption("Vous êtes bien connecté en temps que " +
		                                               client->GetNick());

		bool eob = false;
		SDL_Event event;
		do
		{
			while( SDL_PollEvent( &event) )
			{
				ConnectedForm->Actions(event);
				switch(event.type)
				{
					case SDL_KEYUP:
						switch (event.key.keysym.sym)
						{
							case SDLK_ESCAPE:
								client->SetWantDisconnect();
								client->sendrpl(client->rpl(EC_Client::BYE));
								eob = true;
								break;
							default: break;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
						if(ConnectedForm->DisconnectButton->Test(event.button.x, event.button.y))
						{
							client->SetWantDisconnect();
							client->sendrpl(client->rpl(EC_Client::BYE));
							eob = true;
						}
						if(ConnectedForm->ListButton->Test(event.button.x, event.button.y))
						{
							ListGames();
							client->sendrpl(client->rpl(EC_Client::STAT));
						}
						if(ConnectedForm->CreateButton->Test(event.button.x, event.button.y))
						{
							if(!GameInfos(NULL, ConnectedForm))
							{
								TMessageBox mb("Impossible de créer le salon.\n"
												"Son nom est peut être déjà utilisé.",
												BT_OK, ConnectedForm);
								mb.Show();
							}
							client->sendrpl(client->rpl(EC_Client::STAT));
						}
						break;
					default:
						break;
				}
			}
			ConnectedForm->Update();

			if(!client || !client->IsConnected())
			{
				eob = true;

				TMessageBox mb("Vous avez été déconnecté.", BT_OK, NULL);
				mb.SetBackGround(Resources::Titlescreen());
				mb.Show();
			}
		} while(!eob);
	}
	catch(const TECExcept &e)
	{
		vDebug(W_ERR|W_SEND, e.Message(), e.Vars());
		EC_Client::GetInstance()->SetWantDisconnect();
		TMessageBox mb("Une erreur s'est produite dans le jeu !!\n\n"
		               "Elle a été envoyée aux programmeurs du jeu qui feront leur "
		               "possible pour le corriger.\n\n"
		               "Veuillez nous excuser de la gêne occasionée", BT_OK, NULL);
				mb.SetBackGround(Resources::Titlescreen());
				mb.Show();
	}
	if(EC_Client::GetInstance())
	{
		delete EC_Client::GetInstance();
		EC_Client::singleton = NULL;
	}
	SDL_WaitThread(Thread, 0);

	MyFree(ConnectedForm);

	if(mutex)
	{
		SDL_DestroyMutex(mutex);
		mutex = 0;
	}

	return;
}

/********************************************************************************************
 *                               TConnectedForm                                             *
 ********************************************************************************************/

TConnectedForm::TConnectedForm(ECImage* w)
	: TForm(w)
{
	Motd = AddComponent(new TMemo(Font::GetInstance(Font::Small), 0, 0, 500,350, 0));
	Motd->SetXY(Window()->GetWidth()/2 - Motd->Width()/2, Window()->GetHeight()/2 - Motd->Height()/2);

	Welcome = AddComponent(new TLabel(Motd->Y()-40,"Vous êtes bien connecté", white_color, Font::GetInstance(Font::Big)));

	int button_x = Motd->X() + Motd->Width() + 15;

	CreateButton = AddComponent(new TButtonText(button_x, Motd->Y(), 150,50, "Créer une partie",
	                                            Font::GetInstance(Font::Normal)));
	ListButton = AddComponent(new TButtonText(button_x,CreateButton->Y()+CreateButton->Height(),150,50, "Lister les parties",
	                                          Font::GetInstance(Font::Normal)));
	DisconnectButton = AddComponent(new TButtonText(button_x,ListButton->Y()+ListButton->Height(),150,50, "Se déconnecter",
	                                                Font::GetInstance(Font::Normal)));

	Uptime =    AddComponent(new TLabel(75,Window()->GetHeight()-90,"x", white_color, Font::GetInstance(Font::Normal)));
	UserStats = AddComponent(new TLabel(75,Uptime->Y()+Uptime->Height(),"y", white_color, Font::GetInstance(Font::Normal)));
	ChanStats = AddComponent(new TLabel(75,UserStats->Y()+UserStats->Height(),"z", white_color,
	                                    Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
