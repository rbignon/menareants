/* src/Login.cpp - Login commands
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
#include "Timer.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

TConnectedForm  *ConnectedForm  = NULL;
TListServerForm *ListServerForm = NULL;
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

/** This server rejects me because it is full.
 *
 * Syntax: ER3
 */
int ER3Command::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->SetCantConnect("Le serveur est plein");
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
			                   "sur ce serveur.\n\n"
#ifndef WIN32
			                   "Tapez en console \"make update && sudo make install\".\n"
#else
			                   "Téléchargez MenAreAnts.zip sur le site.\n"
#endif
			                   "Allez sur " APP_SITE " pour plus d'informations.");
			break;
		default:
			me->SetCantConnect("Impossible de se connecter");
			vDebug(W_DESYNCH|W_SEND, "Reception d'un MAJ bizarre !",
		                                VName(parv[1]));
			break;
	}
	return 0;
}

/** I've been disconnected from server when I was playing a game. It asks me if I want to rejoin this game.
 *
 * Syntax: REJOIN game
 */
int REJOINCommand::Exec(PlayerList, EC_Client* me, ParvList parv)
{
	if(!ConnectedForm) return 0;

	ConnectedForm->Rejoin = parv[1];

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
	if(ConnectedForm)
		ConnectedForm->Motd->ScrollUp();
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

/** Connected to a meta-server
 *
 * Syntax: HEL program pversion
 */
int HELmsCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->SetConnected();
	me->sendrpl("IAM %s " CLIENT_SMALLNAME " " APP_MSPROTO, Config::GetInstance()->nick.c_str());
	me->UnsetLogging();
	return 0;
}

/** Receive a server name in the list from meta-server
 *
 * Syntax: LSP ip nom +/- nbjoueurs nbmax nbgames maxgames nbwgames proto
 */
int LSPmsCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	int sock;
	static struct sockaddr_in fsocket_init;
	struct sockaddr_in fsocket = fsocket_init;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::string host = parv[1];

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(stringtok(host, ":").c_str());
	fsocket.sin_port = htons(host.empty() ? SERV_DEFPORT : StrToTyp<uint>(host));

	Timer timer;
	int t0 = SDL_GetTicks();

	/* Connexion */
	if(connect(sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
		return 0;

	ListServerForm->ServerList->AddItem(false,
	                      StringF("%4d %-27s %2s    %3s/%-3s    %3s/%-3s       %-3s", SDL_GetTicks()-t0, parv[2].c_str(), parv[9].c_str(),
	                              parv[4].c_str(), parv[5].c_str(), parv[6].c_str(), parv[7].c_str(), parv[8].c_str()),
	                      parv[1], (parv[3][0] == '+' && parv[9] == APP_PVERSION) ? black_color : red_color,
	                               (parv[3][0] == '+' && parv[9] == APP_PVERSION));
	close(sock);
	return 0;
}

int EOLmsCommand::Exec(PlayerList players, EC_Client* me, ParvList parv)
{
	ListServerForm->RecvSList = true;
	return 0;
}

void MenAreAntsApp::RefreshList()
{
	ListServerForm->RecvSList = false;
	mutex = SDL_CreateMutex();
	EC_Client* client = EC_Client::GetInstance(true);

	client->SetMutex(mutex);
	client->SetHostName(Config::GetInstance()->hostname);
	client->SetPort(Config::GetInstance()->port);
	/* Ajout des commandes            CMDNAME FLAGS ARGS */
	client->AddCommand(new HELmsCommand("HEL",	0,	1));
	client->AddCommand(new LSPmsCommand("LSP",	0,	1));
	client->AddCommand(new EOLmsCommand("EOL",	0,	0));

	Thread = SDL_CreateThread(EC_Client::read_sock, client);

	ListServerForm->ServerList->ClearItems();

	WAIT_EVENT_T(client->IsConnected() || client->Error() || ListServerForm->RecvSList, i, 5);

	if((!client || !client->IsConnected()) && !ListServerForm->RecvSList)
	{
		std::string msg;
		if(client)
			client->SetWantDisconnect();

		if(!client)
			msg = "Connexion impossible au meta-serveur";
		else
			msg = "Connexion impossible au meta-serveur :\n\n" + client->CantConnect();
		TMessageBox(msg, BT_OK, ListServerForm).Show();

		SDL_WaitThread(Thread, 0);
		delete client;
		EC_Client::singleton = 0;
		if(mutex)
		{
			SDL_DestroyMutex(mutex);
			mutex = 0;
		}
		ListServerForm->RecvSList = false;
		return;
	}

	ListServerForm->SetMutex(mutex);
}

void MenAreAntsApp::ServerList()
{
	ListServerForm = new TListServerForm(Video::GetInstance()->Window());

	SDL_Event event;
	bool eob = false;

	RefreshList();
	Timer timer;
	do
	{
		if(timer.time_elapsed() > 60)
		{
			RefreshList();
			timer.reset();
		}
		if(ListServerForm->RecvSList && EC_Client::GetInstance() != 0 && EC_Client::GetInstance()->IsConnected() == false)
		{
			EC_Client::GetInstance()->SetWantDisconnect();
			SDL_WaitThread(Thread, 0);
			delete EC_Client::GetInstance();
			EC_Client::singleton = NULL;
			mutex = 0;
			ListServerForm->SetMutex(0);
			ListServerForm->RecvSList = false;
		}
		while(SDL_PollEvent(&event))
		{
			ListServerForm->Actions(event);
			switch(event.type)
			{
				case SDL_MOUSEBUTTONDOWN:
					if(ListServerForm->ServerList->Test(event.button.x, event.button.y, event.button.button))
					{
						if(ListServerForm->ServerList->GetSelectedItem() >= 0 &&
						ListServerForm->ServerList->EnabledItem(ListServerForm->ServerList->GetSelectedItem()))
							ListServerForm->ConnectButton->SetEnabled(true);
						else
							ListServerForm->ConnectButton->SetEnabled(false);
					}
					if(ListServerForm->RefreshButton->Test(event.button.x, event.button.y, event.button.button))
						RefreshList();
					else if(ListServerForm->ConnectButton->Test(event.button.x, event.button.y, event.button.button))
						ConnectedTo(ListServerForm->ServerList->ReadValue(
						             ListServerForm->ServerList->GetSelectedItem()));
					else if(ListServerForm->RetourButton->Test(event.button.x, event.button.y, event.button.button))
						eob = true;
					break;
				default: break;
			}
		}
		ListServerForm->Update();
	} while(!eob);
	MyFree(ListServerForm);
}

/*********************************************************************************************
 *                               TListServerForm                                             *
 ********************************************************************************************/

TListServerForm::TListServerForm(ECImage* w)
	: TForm(w)
{
	ServerList = AddComponent(new TListBox(Font::GetInstance(Font::Small), 0, 0, 500,350));
	ServerList->SetXY(Window()->GetWidth()/2 - ServerList->Width()/2 - 50, Window()->GetHeight()/2 - ServerList->Height()/2);

	Label1 = AddComponent(new TLabel(ServerList->Y()-60,"Veuillez choisir un serveur dans la liste suivante :", white_color, Font::GetInstance(Font::Big)));

	Label2 = AddComponent(new TLabel(ServerList->X(), ServerList->Y()-20, "", white_color, Font::GetInstance(Font::Normal)));
	Label2->SetCaption("Ping  Name                 Proto  Joueurs  Parties  En attente");

	int button_x = ServerList->X() + ServerList->Width() + 15;

	ConnectButton = AddComponent(new TButtonText(button_x, ServerList->Y(), 150,50, "Se connecter",
	                                            Font::GetInstance(Font::Normal)));
	ConnectButton->SetEnabled(false);
	RefreshButton = AddComponent(new TButtonText(button_x, ConnectButton->Y()+ConnectButton->Height(), 150,50, "Actualiser",
	                                            Font::GetInstance(Font::Normal)));
	RetourButton = AddComponent(new TButtonText(button_x,RefreshButton->Y()+RefreshButton->Height(),150,50, "Retour",
	                                          Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}

void MenAreAntsApp::ConnectedTo(std::string host)
{
	if(EC_Client::GetInstance())
	{
		FDebug(W_SEND|W_ERR, "Appel en étant déjà connecté !");
		return;
	}

	EC_Client* client = 0;

	try
	{

		ConnectedForm = new TConnectedForm(Video::GetInstance()->Window());

		mutex = SDL_CreateMutex();
		client = EC_Client::GetInstance(true);

		client->SetMutex(mutex);
		client->SetHostName(stringtok(host, ":"));
		client->SetPort(host.empty() ? SERV_DEFPORT : StrToTyp<uint>(host));
		/* Ajout des commandes            CMDNAME FLAGS ARGS */
		client->AddCommand(new ARMCommand("ARM",	0,	0));

		client->AddCommand(new PIGCommand("PIG",	0,	0));
		client->AddCommand(new HELCommand("HEL",	0,	1));
		client->AddCommand(new AIMCommand("AIM",	0,	1));
		client->AddCommand(new USEDCommand("USED",	0,	0));
		client->AddCommand(new MAJCommand("MAJ",	0,	1));
		client->AddCommand(new MOTDCommand("MOTD",	0,	0));
		client->AddCommand(new EOMCommand("EOM",	0,	0));
		client->AddCommand(new STATCommand("STAT",	0,	7));
		client->AddCommand(new ER1Command("ER1",	0,	0));
		client->AddCommand(new ER2Command("ER2",	0,	1));
		client->AddCommand(new ER3Command("ER3",	0,	0));
		client->AddCommand(new ER4Command("ER4",	0,	0));

		client->AddCommand(new LSPCommand("LSP",	0,	3));
		client->AddCommand(new EOLCommand("EOL",	0,	0));

		client->AddCommand(new BPCommand("BP",		0,	1));

		client->AddCommand(new JOICommand("JOI",	0,	1));
		client->AddCommand(new SETCommand("SET",	0,	1));
		client->AddCommand(new PLSCommand("PLS",	0,	1));
		client->AddCommand(new LEACommand("LEA",	0,	0));
		client->AddCommand(new KICKCommand("KICK",	0,	1));
		client->AddCommand(new MSGCommand("MSG",	0,	1));
		client->AddCommand(new INFOCommand("INFO",	0,	1));

		client->AddCommand(new LSMCommand("LSM",	0,	3));
		client->AddCommand(new EOMAPCommand("EOMAP",0,	0));
		client->AddCommand(new SMAPCommand("SMAP",	0,	1));
		client->AddCommand(new EOSMAPCommand("EOSMAP",0,0));

		client->AddCommand(new SCOCommand("SCO",	0,	4));
		client->AddCommand(new REJOINCommand("REJOIN",0,1));

		Thread = SDL_CreateThread(EC_Client::read_sock, client);

		WAIT_EVENT_T(client->IsConnected() || client->Error(), i, 5);

		if(!client || !client->IsConnected())
		{
			std::string msg;
			if(client)
				client->SetWantDisconnect();

			if(!client)
				msg = "Connexion impossible à " + host;
			else
				msg = "Connexion impossible à " + host + " :\n\n" + client->CantConnect();
			TMessageBox(msg, BT_OK, ListServerForm).Show();

			SDL_WaitThread(Thread, 0);
			delete client;
			EC_Client::singleton = 0;
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
								eob = true;
								break;
							default: break;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
						if(ConnectedForm->DisconnectButton->Test(event.button.x, event.button.y, event.button.button))
						{
							client->SetWantDisconnect();
							eob = true;
						}
						if(ConnectedForm->ListButton->Test(event.button.x, event.button.y, event.button.button))
						{
							ListGames();
							client->sendrpl(client->rpl(EC_Client::STAT));
						}
						if(ConnectedForm->MissionButton->Test(event.button.x, event.button.y, event.button.button))
						{
							if(!GameInfos(NULL, ConnectedForm, true))
								TMessageBox("Impossible de créer une mission.\n"
								            "Il y a actuellement de trop de parties en cours sur le serveur.",
								            BT_OK, ConnectedForm).Show();

							client->sendrpl(client->rpl(EC_Client::STAT));
						}
						if(ConnectedForm->EscarmoucheButton->Test(event.button.x, event.button.y, event.button.button))
						{
							if(!GameInfos(NULL, ConnectedForm))
								TMessageBox("Impossible de créer une escarmouche.\n"
								            "Il y a actuellement de trop de parties en cours sur le serveur.",
								            BT_OK, ConnectedForm).Show();
							client->sendrpl(client->rpl(EC_Client::STAT));
						}
						break;
					default:
						break;
				}
			}
			ConnectedForm->Update();

			if(ConnectedForm->Rejoin.empty() == false)
			{
				if(TMessageBox("Vous avez été déconnecté pendant que vous jouiez à la partie " +
				                ConnectedForm->Rejoin + ".\n\n"
				                "Souhaitez-vous rejoindre la partie ?",
				               BT_YES|BT_NO, ConnectedForm).Show() == BT_YES)
				{
					RecoverGame(ConnectedForm->Rejoin);
				}
				ConnectedForm->Rejoin.clear();
			}
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
#ifdef DEBUG
		throw;
#endif
	}
	if(client)
	{
		SDL_WaitThread(Thread, 0);
		delete client;
		EC_Client::singleton = NULL;
	}
	else if(Thread)
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

	MissionButton = AddComponent(new TButtonText(button_x, Motd->Y(), 150,50, "Missions",
	                                            Font::GetInstance(Font::Normal)));
	EscarmoucheButton = AddComponent(new TButtonText(button_x, MissionButton->Y()+MissionButton->Height(), 150,50, "Escarmouche",
	                                            Font::GetInstance(Font::Normal)));
	ListButton = AddComponent(new TButtonText(button_x,EscarmoucheButton->Y()+EscarmoucheButton->Height(),150,50, "Multijoueur",
	                                          Font::GetInstance(Font::Normal)));
	DisconnectButton = AddComponent(new TButtonText(button_x,ListButton->Y()+ListButton->Height(),150,50, "Se déconnecter",
	                                                Font::GetInstance(Font::Normal)));

	Uptime =    AddComponent(new TLabel(75,Window()->GetHeight()-90," ", white_color, Font::GetInstance(Font::Normal)));
	UserStats = AddComponent(new TLabel(75,Uptime->Y()+Uptime->Height()," ", white_color, Font::GetInstance(Font::Normal)));
	ChanStats = AddComponent(new TLabel(75,UserStats->Y()+UserStats->Height()," ", white_color,
	                                    Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
