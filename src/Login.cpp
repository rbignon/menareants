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

#include <errno.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "Commands.h"
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
#include "JoinGame.h"
#include "Sockets.h"

TConnectedForm  *ConnectedForm  = NULL;
TListServerForm *ListServerForm = NULL;
static bool RC_MOTD = false;
bool EOL = false;                     /**< EOL is setted to \a true by thread when it received all list of games */

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
		ConnectedForm->Uptime->SetCaption("Ce serveur a été lancé il y a " +
		                                  std::string(duration(time(NULL) - StrToTyp<time_t>(parv[7]))) + ".");
		ConnectedForm->UserStats->SetCaption("Il y a " + parv[1] + " personnes connectées, avec " + parv[2] +
		                                     " connexions totales et " + parv[6] + " parties jouées.");
		ConnectedForm->ChanStats->SetCaption("Il y a actuellement " + parv[3] + " partie(s), dont " + parv[5] +
		                                     " en cours de jeu et " + parv[4] + " en préparation.");
	}
	return 0;
}

/** List games.
 *
 * Syntax: LSP nom ingame nbjoueur nbmax [mapname]
 */
int LSPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!ConnectedForm)
		vDebug(W_DESYNCH|W_SEND, "Reception d'un LSP hors de la fenêtre de liste des chans", VPName(ConnectedForm));

	me->LockScreen();
	if(parv[2][0] == '+')
	{
		if(parv[3] == "0")
			ConnectedForm->GList->AddItem(false, StringF("%-8s %2s", parv[1].c_str(), parv[2].c_str()), parv[1],
			                              black_color, true);
		else if(parv.size() > 5)
			ConnectedForm->GList->AddItem(false, StringF("%-8s %2s/%-2s %s", parv[1].c_str(), parv[3].c_str(),
			                                                                 parv[4].c_str(), parv[5].c_str()), parv[1],
			                              (parv.size() <= 4 || parv[3] != parv[4]) ? black_color : red_color,
			                              (parv.size() <= 4 || parv[3] != parv[4]) ? true : false);
		else
			ConnectedForm->GList->AddItem(false, StringF("%-8s %2s/%-2s", parv[1].c_str(), parv[3].c_str(),
			                                                              parv[4].c_str()), parv[1],
			                              (parv.size() <= 4 || parv[3] != parv[4]) ? black_color : red_color,
			                              (parv.size() <= 4 || parv[3] != parv[4]) ? true : false);
	}
	else
		ConnectedForm->GList->AddItem(false, StringF("%-8s  Playing: %s", parv[1].c_str(), parv[5].c_str()), parv[1],
		                             red_color, false);
	me->UnlockScreen();
	return 0;
}

/** End of channel list
 *
 * Syntax: EOL
 */
int EOLCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	EOL = true;
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
 *
 * parv[1] = ip
 * parv[2] = name
 * parv[3] = +/-
 * parv[4] = nbplayers
 * parv[5] = maxplayers
 * parv[6] = nbgames
 * parv[7] = maxgames
 * parv[8] = nbwgames
 * parv[9] = proto
 */
int LSPmsCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	assert(ListServerForm);

	int sock;
	static struct sockaddr_in fsocket_init;
	struct sockaddr_in fsocket = fsocket_init;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::string host = parv[1];

	// Set the timeout
	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 5; // 5seconds timeout
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(stringtok(host, ":").c_str());
	fsocket.sin_port = htons(host.empty() ? SERV_DEFPORT : StrToTyp<uint>(host));

	Timer timer;
	int t0 = SDL_GetTicks();

	/* Connexion */
	if(connect(sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
	{
		Debug(W_DEBUG, "Unable to connect to %s (%d: %s)", parv[2].c_str(), errno, strerror(errno));
		return 0;
	}

	me->LockScreen();
	if(parv[3][0] == '+' && parv[9] == APP_PVERSION && StrToTyp<int>(parv[4]) > 0)
		ListServerForm->ServerList->AddItem(false,
		                      StringF("%3d  %-23s %2s   %3s/%-3s   %3s/%-3s     %-3s", SDL_GetTicks()-t0, parv[2].substr(0,23).c_str(), parv[9].c_str(),
		                              parv[4].c_str(), parv[5].c_str(), parv[6].c_str(), parv[7].c_str(), parv[8].c_str()),
		                      parv[1], fgreen_color, true, *Font::GetInstance(Font::Normal));
	else
		ListServerForm->ServerList->AddItem(false,
		                      StringF("%4d %-27s %2s    %3s/%-3s    %3s/%-3s       %-3s", SDL_GetTicks()-t0, parv[2].c_str(), parv[9].c_str(),
		                              parv[4].c_str(), parv[5].c_str(), parv[6].c_str(), parv[7].c_str(), parv[8].c_str()),
		                      parv[1], (parv[3][0] == '+' && parv[9] == APP_PVERSION) ? black_color : red_color,
		                      (parv[3][0] == '+' && parv[9] == APP_PVERSION));

	ListServerForm->nb_chans += StrToTyp<uint>(parv[6]);
	ListServerForm->nb_wchans += StrToTyp<uint>(parv[8]);
	ListServerForm->nb_users += StrToTyp<uint>(parv[4]);
	me->UnlockScreen();
#ifdef WIN32
	closesocket(sock);
#else
	close(sock);
#endif
	return 0;
}

int EOLmsCommand::Exec(PlayerList players, EC_Client* me, ParvList parv)
{
	assert(ListServerForm);
	ListServerForm->RecvSList = false;
	ListServerForm->UserStats->SetCaption("Il y a " + TypToStr(ListServerForm->nb_users) + " utilisateur(s), avec " + TypToStr(ListServerForm->nb_tusers) + " connexions totales et " +
	                                     TypToStr(ListServerForm->nb_tchans) + " parties jouées.");
	ListServerForm->ChanStats->SetCaption("Il y a actuellement " + TypToStr(ListServerForm->nb_chans) + " partie(s) sur " + TypToStr(ListServerForm->ServerList->Size()) +
	                                      " serveurs, dont " + TypToStr(ListServerForm->nb_chans - ListServerForm->nb_wchans) + " en cours de jeu et " +
	                                      TypToStr(ListServerForm->nb_wchans) + " en préparation.");
	ListServerForm->nb_chans = ListServerForm->nb_wchans = ListServerForm->nb_users = ListServerForm->nb_tchans = ListServerForm->nb_tusers = 0;
	return 0;
}

/** Statistics about meta-server
 *
 * Syntax: STAT chans users
 */
int STATmsCommand::Exec(PlayerList players, EC_Client* me, ParvList parv)
{
	assert(ListServerForm);
	ListServerForm->nb_tchans = StrToTyp<uint>(parv[1]);
	ListServerForm->nb_tusers = StrToTyp<uint>(parv[2]);
	return 0;
}

void MenAreAntsApp::RefreshList()
{
	if(ListServerForm->RecvSList == true || EC_Client::GetInstance()) return;

	ListServerForm->RecvSList = true;
	ListServerForm->RefreshButton->SetEnabled(false);
	ListServerForm->EscarmoucheButton->SetEnabled(false);
	ListServerForm->MissionButton->SetEnabled(false);

	mutex = SDL_CreateMutex();
	EC_Client* client = EC_Client::GetInstance(true);

	client->SetMutex(mutex);
	client->SetHostName(Config::GetInstance()->hostname);
	client->SetPort(Config::GetInstance()->port);
	/* Ajout des commandes            CMDNAME FLAGS ARGS */
	client->AddCommand(new HELmsCommand("HEL",	0,	1));
	client->AddCommand(new LSPmsCommand("LSP",	0,	1));
	client->AddCommand(new EOLmsCommand("EOL",	0,	0));
	client->AddCommand(new STATmsCommand("STAT",	0,	2));

	Thread = SDL_CreateThread(EC_Client::read_sock, client);

	ListServerForm->ServerList->ClearItems();

	WAIT_EVENT_T(client->IsConnected() || client->Error() || !ListServerForm->RecvSList, i, 5);

	if((!client || !client->IsConnected()) && ListServerForm->RecvSList)
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

	RefreshList();
	do
	{
		if(!ListServerForm->RecvSList && EC_Client::GetInstance() != 0 && EC_Client::GetInstance()->IsConnected() == false)
		{
			EC_Client::GetInstance()->SetWantDisconnect();
			SDL_WaitThread(Thread, 0);
			delete EC_Client::GetInstance();
			EC_Client::singleton = NULL;
			mutex = 0;
			ListServerForm->SetMutex(0);
			ListServerForm->RefreshButton->SetEnabled(true);
			ListServerForm->EscarmoucheButton->SetEnabled(true);
			ListServerForm->MissionButton->SetEnabled(true);
		}
		ListServerForm->Actions();
		ListServerForm->Update();
	} while(!ListServerForm->WantQuit());
	MyFree(ListServerForm);
}

/*********************************************************************************************
 *                               TListServerForm                                             *
 ********************************************************************************************/

void TListServerForm::AfterDraw()
{
	if(timer.time_elapsed() > 60)
	{
		MenAreAntsApp::GetInstance()->RefreshList();
		timer.reset();
	}
}

void TListServerForm::OnClic(const Point2i& mouse, int button, bool& stop)
{
	if(ServerList->Test(mouse, button))
	{
		if(ServerList->Selected() >= 0 && ServerList->SelectedItem()->Enabled())
			ConnectButton->SetEnabled(true);
		else
			ConnectButton->SetEnabled(false);
	}
	if(RefreshButton->Test(mouse, button))
		MenAreAntsApp::GetInstance()->RefreshList();
	else if(ConnectButton->Test(mouse, button))
	{
		MenAreAntsApp::GetInstance()->ConnectedTo(ServerList->SelectedItem()->Value());
		MenAreAntsApp::GetInstance()->RefreshList();
	}
	else if(RetourButton->Test(mouse, button))
		want_quit = true;
	else if(MissionButton->Test(mouse, button) || EscarmoucheButton->Test(mouse, button))
	{
		int size = ServerList->Size();
		if(!size)
		{
			TMessageBox("Il n'y a aucun serveur disponible.\n\nVeuillez réessayer plus tard.",
			            BT_OK, this).Show();
			return;
		}
		int r = 0, i = 0;
		do
		{
			r = rand()%size;
			i++;
		} while(i < 20 && ServerList->Item(r)->Enabled() == false);
		if(i >= 20)
			TMessageBox("Il n'y a aucun serveur pour heberger votre partie.\n\n"
					"Veuillez réessayez plus tard.",
						BT_OK, this).Show();
		else
		{
			EC_Client* client = MenAreAntsApp::GetInstance()->Connect(ServerList->ReadValue(r));
			if(!client)
				return;
			if(!MenAreAntsApp::GetInstance()->GameInfos(NULL, ConnectedForm, MissionButton->Test(mouse, button)))
				TMessageBox("Impossible de créer la partie.\n"
				            "Il y a actuellement de trop de parties en cours sur le serveur. Réessayez.",
				            BT_OK, ConnectedForm).Show();
			if(!client->IsConnected())
				TMessageBox("Vous avez été déconnecté.", BT_OK).Show();
			client->SetWantDisconnect();
			MenAreAntsApp::GetInstance()->Disconnect(client);
			MenAreAntsApp::GetInstance()->RefreshList();
		}
	}
}

TListServerForm::TListServerForm(ECImage* w)
	: TForm(w), nb_chans(0), nb_wchans(0), nb_users(0), nb_tchans(0), nb_tusers(0), RecvSList(false)
{
	ServerList = AddComponent(new TListBox(Rectanglei(0, 0, 500,350)));
	ServerList->SetXY(Window()->GetWidth()/2 - ServerList->Width()/2 - 50, Window()->GetHeight()/2 - ServerList->Height()/2 + 70);
	ServerList->SetGrayDisable(false);

	Label1 = AddComponent(new TLabel(ServerList->Y()-60,"Veuillez choisir un serveur dans la liste suivante :", white_color, Font::GetInstance(Font::Big)));

	Label2 = AddComponent(new TLabel(ServerList->X(), ServerList->Y()-20, "", white_color, Font::GetInstance(Font::Normal)));
	Label2->SetCaption("Ping  Name                 Proto  Joueurs  Parties  En attente");

	MissionButton = AddComponent(new TButton(Window()->GetWidth()/2 - 100, Label1->Y()/2, 220,50));
	MissionButton->SetImage(new ECSprite(Resources::MissionButton(), Video::GetInstance()->Window()));

	if(Label1->Y() - (MissionButton->Y()+MissionButton->Height()) <= 50)
	{
		MissionButton->SetX(MissionButton->X() - 200);
		EscarmoucheButton = AddComponent(new TButton(MissionButton->X()+MissionButton->Width()+100, MissionButton->Y(), 220,50));
	}
	else
		EscarmoucheButton = AddComponent(new TButton(MissionButton->X(), MissionButton->Y()+MissionButton->Height()+(Label1->Y() - (MissionButton->Y()+MissionButton->Height()))/2-25, 220,50));

	EscarmoucheButton->SetImage(new ECSprite(Resources::EscarmoucheButton(), Video::GetInstance()->Window()));

	UserStats = AddComponent(new TLabel(ServerList->X()-25, ServerList->Y()+ServerList->Height()+10, " ", white_color, Font::GetInstance(Font::Normal)));

	ChanStats = AddComponent(new TLabel(ServerList->X()-25, UserStats->Y()+UserStats->Height(), " ", white_color, Font::GetInstance(Font::Normal)));

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

EC_Client* MenAreAntsApp::Connect(std::string host)
{
	mutex = SDL_CreateMutex();
	EC_Client* client = EC_Client::GetInstance(true);

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
		return 0;
	}

	return client;
}

void MenAreAntsApp::Disconnect(EC_Client* client)
{
	if(client)
	{
		client->SetWantDisconnect();
		SDL_WaitThread(Thread, 0);
		delete client;
		EC_Client::singleton = NULL;
	}
	else if(Thread)
		SDL_WaitThread(Thread, 0);

	if(mutex)
	{
		SDL_DestroyMutex(mutex);
		mutex = 0;
	}
}

void MenAreAntsApp::ConnectedTo(std::string host)
{
	if(EC_Client::GetInstance())
	{
		FDebug(W_SEND|W_ERR, "We are already connected !");
		return;
	}

	EC_Client* client = 0;

	try
	{

		ConnectedForm = new TConnectedForm(Video::GetInstance()->Window());

		if(!(client = Connect(host)))
			return;

		ConnectedForm->SetMutex(mutex);
		ConnectedForm->SetClient(client);
		ConnectedForm->Welcome->SetCaption("Vous êtes bien connecté en tant que " +
		                                               client->GetNick());

		ConnectedForm->Run();
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
	MyFree(ConnectedForm);

	Disconnect(client);

	return;


}

/********************************************************************************************
 *                               TConnectedForm                                             *
 ********************************************************************************************/

void TConnectedForm::AfterDraw()
{
	if(Rejoin.empty() == false)
	{
		if(TMessageBox("Vous avez été déconnecté pendant que vous jouiez à la partie " +
		               Rejoin + ".\n\n"
		               "Souhaitez-vous rejoindre la partie ?",
				BT_YES|BT_NO, this).Show() == BT_YES)
		{
			MenAreAntsApp::GetInstance()->RecoverGame(Rejoin);
		}
		Rejoin.clear();
	}
	if(!client || !client->IsConnected())
	{
		TMessageBox mb("Vous avez été déconnecté.", BT_OK, NULL);
		mb.SetBackGround(Resources::Titlescreen());
		mb.Show();
		want_quit = true;
		return;
	}
	if(timer.time_elapsed(true) > 60 || refresh)
	{
		GList->ClearItems();
		EOL = false;
		client->sendrpl(client->rpl(EC_Client::LISTGAME));
		WAIT_EVENT(EOL, j);
		if(!EOL)
			return;

		refresh = false;
		JoinButton->SetEnabled(false);
		timer.reset();
	}
}

void TConnectedForm::OnClic(const Point2i& mouse, int button, bool&)
{
	if(GList->Test(mouse, button))
	{
		if(GList->Selected() >= 0 && GList->SelectedItem()->Enabled())
			JoinButton->SetEnabled(true);
		else
			JoinButton->SetEnabled(false);
	}
	else if(DisconnectButton->Test(mouse, button))
	{
		client->SetWantDisconnect();
		want_quit = true;
	}
	else if(JoinButton->Enabled() && JoinButton->Test(mouse, button))
	{
		if(!MenAreAntsApp::GetInstance()->GameInfos(GList->SelectedItem()->Value().c_str(), this))
		{
			TMessageBox(TGameInfosForm::ErrMessage, BT_OK, this).Show();
			TGameInfosForm::ErrMessage.clear();
		}
		refresh = true;
		timer.reset();
		client->sendrpl(client->rpl(EC_Client::STAT));
	}
	else if(RefreshButton->Test(mouse, button))
		refresh = true;
	else if(CreerButton->Test(mouse, button))
	{
		if(!MenAreAntsApp::GetInstance()->GameInfos(NULL, this))
		{
			TMessageBox(TGameInfosForm::ErrMessage, BT_OK, this).Show();
			TGameInfosForm::ErrMessage.clear();
		}
		refresh = true;
		timer.reset();
		client->sendrpl(client->rpl(EC_Client::STAT));
	}

}

TConnectedForm::TConnectedForm(ECImage* w)
	: TForm(w), refresh(true), client(0)
{
	Motd = AddComponent(new TMemo(Font::GetInstance(Font::Small), 0, 0, 400,350, 0));
	Motd->SetXY(Window()->GetWidth()/2 - Motd->Width()/2 - (200+150+10+10)/2, Window()->GetHeight()/2 - Motd->Height()/2);

	MOTDLabel = AddComponent(new TLabel(Motd->X()+5, Motd->Y()-20, "Message du jour du serveur :", white_color, Font::GetInstance(Font::Normal)));

	GList = AddComponent(new TListBox(Rectanglei(300,350,200,350)));
	GList->SetXY(Motd->X()+Motd->Width()+10, Motd->Y());
	GList->SetGrayDisable(false);

	ListLabel = AddComponent(new TLabel(GList->X()+5, GList->Y()-20, "Liste des parties :", white_color, Font::GetInstance(Font::Normal)));

	Welcome = AddComponent(new TLabel(Motd->Y()/2,"Vous êtes bien connecté", white_color, Font::GetInstance(Font::Big)));

	int button_x = GList->X() + GList->Width() + 10;

	JoinButton = AddComponent(new TButtonText(button_x, GList->Y(), 150,50, "Rejoindre",
	                                            Font::GetInstance(Font::Normal)));
	JoinButton->SetEnabled(false);
	CreerButton = AddComponent(new TButtonText(button_x,JoinButton->Y()+JoinButton->Height(), 150,50, "Créer une partie",
	                                            Font::GetInstance(Font::Normal)));
	RefreshButton = AddComponent(new TButtonText(button_x,CreerButton->Y()+CreerButton->Height(),150,50, "Actualiser",
	                                                Font::GetInstance(Font::Normal)));
	DisconnectButton = AddComponent(new TButtonText(button_x,RefreshButton->Y()+RefreshButton->Height(),150,50, "Se déconnecter",
	                                                Font::GetInstance(Font::Normal)));

	Uptime =    AddComponent(new TLabel(75,Window()->GetHeight()-90," ", white_color, Font::GetInstance(Font::Normal)));
	UserStats = AddComponent(new TLabel(75,Uptime->Y()+Uptime->Height()," ", white_color, Font::GetInstance(Font::Normal)));
	ChanStats = AddComponent(new TLabel(75,UserStats->Y()+UserStats->Height()," ", white_color,
	                                    Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
