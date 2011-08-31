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

TConnectedForm    *ConnectedForm  = NULL;
TListServerForm   *ListServerForm = NULL;
TGlobalScoresForm *GlobalScoresForm = NULL;
static bool RC_MOTD = false;
extern bool JOINED;
bool EOL = false;                     /**< EOL is setted to \a true by thread when it received all list of games */

/** This is a handler for errors messages.
 *
 * Syntax: ERROR number [args]
 */
int ERRORCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	ECError err = static_cast<ECError>(parv[1][0]);
	std::string ErrMessage;

	switch(err)
	{
		case ERR_UNKNOWN:
			break;
		case ERR_NICK_USED:
			me->SetCantConnect(StringF(_("%s is already used"), Config::GetInstance()->nick.c_str()));
			break;
		case ERR_BANNED:
			me->SetCantConnect(_("You are banned from this server.\n\n") + parv[2]);
			break;
		case ERR_CANT_JOIN:
			JOINED = -1;
			ErrMessage = _("Unable to join this game. It doesn't exist anymore, or it is full, or it had started");
			break;
		case ERR_IA_CANT_JOIN:
			ErrMessage = _("Unable to create this AI because of one of these reasons:\n"
			             "- Nickname specified is incorrect.\n"
			             "- Nickname specified is alreayd used by an other AI (here or in an other game).\n"
			             "- This game is full.");
			break;
		case ERR_SERV_FULL:
			me->SetCantConnect(_("This server is full."));
			break;
		case ERR_CANT_CREATE:
			JOINED = -1;
			ErrMessage = _("Unable to create a game, there are too games on this server");
			break;
		case ERR_CMDS:
			ErrMessage = _("Server doesn't understand a command we have sended");
			break;
		case ERR_ADMIN_LOGFAIL:
			ErrMessage = _("Incorrect password.");
			break;
		case ERR_ADMIN_NOSUCHVICTIM:
			ErrMessage = StringF(_("No such %s nickname"), parv[2].c_str());
			break;
		case ERR_ADMIN_CANT_REHASH:
			ErrMessage = _("Server can't rehash configuration. Please verify it.");
			break;
		case ERR_ADMIN_SUCCESS:
			ErrMessage = ("Done.");
			ConnectedForm->RehashButton->SetVisible(true);
			ConnectedForm->KillButton->SetVisible(true);
			break;
		case ERR_REGNICK:
			if(Config::GetInstance()->passwd.empty() == false)
				me->sendrpl(MSG_LOGIN, Config::GetInstance()->passwd);
			else if(ListServerForm)
				ListServerForm->login = true;
			break;
		case ERR_LOGIN_SUCCESS:
			if(!ListServerForm) break;
			if(ListServerForm->login)
			{
				ListServerForm->login = false;
				ErrMessage = _("You have logged.");
			}
			MenAreAntsApp::GetInstance()->FirstRunDone();
			ListServerForm->RegisterButton->Hide();
			ListServerForm->AccountButton->Show();
			break;
		case ERR_LOGIN_BADPASS:
			Config::GetInstance()->passwd.clear();
			MetaServer.SetWantDisconnect();
			ErrMessage = _("Incorrect password.");
			if(ListServerForm)
			{
				ListServerForm->SetWantQuit();
				ListServerForm->login = false;
			}
			break;
	}
	if(!ErrMessage.empty())
		TForm::Message = ErrMessage;
	return 0;
}


char *duration(int s)
{
        static char dur[44 /* 3 + 7 + 2 + 8 + 2 + 9 + 2 + 9 + 1 */];
        int i = 0;

        if(s >= 86400)
                i += snprintf(dur + i, sizeof dur - i, "%d", s/86400),
                                s %= 86400, strcpy(dur + i, _(" days ")), i += 7;
        if(s >= 3600)
                i += snprintf(dur + i, sizeof dur - i, "%d", s/3600),
                                s %= 3600, strcpy(dur + i, _(" hours ")), i += 8;
        if(s >= 60)
                i += snprintf(dur + i, sizeof dur - i, "%d", s/60),
                                s %= 60, strcpy(dur + i, _(" minutes ")), i += 9;
        if(s) i += snprintf(dur + i, sizeof dur - i, _("%d seconds"),s);
        else dur[i-2]= 0;

        return dur;
}

void MenAreAntsApp::RefreshList()
{
	ListServerForm->ConnectButton->SetEnabled(false);
	ListServerForm->RefreshButton->SetEnabled(false);
	ListServerForm->EscarmoucheButton->SetEnabled(false);
	ListServerForm->MissionButton->SetEnabled(false);
	ListServerForm->StatsButton->SetEnabled(false);
	ListServerForm->RegisterButton->SetEnabled(false);
	ListServerForm->AccountButton->SetEnabled(false);
	ListServerForm->RetourButton->SetEnabled(false);
	ListServerForm->ServerList->ClearItems();

	if(!MetaServer.Request(MSG_SERVLIST))
	{
		TMessageBox(_("Unable to connect to meta-server"), BT_OK, ListServerForm).Show();
		ListServerForm->SetWantQuit();
	}
}

void MenAreAntsApp::ServerList()
{
	ListServerForm = new TListServerForm(Video::GetInstance()->Window());

	EC_Client* client = &MetaServer;

	client->ClearCommands();
	/* Ajout des commandes            CMDNAME FLAGS ARGS */
	client->AddCommand(new PIGCommand(MSG_PING,		0,	0)); // on appelle volontairement PIGCommand() qui fait la même chose pour serveur et meta-serveur
	client->AddCommand(new AIMCommand(MSG_LOGGED,		0,	1));
	client->AddCommand(new LSPmsCommand(MSG_SERVLIST,	0,	1));
	client->AddCommand(new EOLmsCommand(MSG_ENDOFSLIST,	0,	0));
	client->AddCommand(new STATmsCommand(MSG_STAT,		0,	2));
	client->AddCommand(new REJOINmsCommand(MSG_REJOIN,	0,	1));
	client->AddCommand(new HELmsCommand(MSG_HELLO,		0,	1));
	client->AddCommand(new ERRORCommand(MSG_ERROR,		0,	1));
	client->AddCommand(new SCOREmsCommand(MSG_SCORE,	0,	6));

	client->SetHostName(Config::GetInstance()->hostname);
	client->SetPort(Config::GetInstance()->port);

	RefreshList();

	ListServerForm->Run();

	client->SetWantDisconnect();

	MyFree(ListServerForm);
}

/*********************************************************************************************
 *                               TListServerForm                                             *
 ********************************************************************************************/

/** Connected to a meta-server
 *
 * Syntax: HEL program pversion
 */
int HELmsCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	ECArgs args = Config::GetInstance()->nick;
	args += CLIENT_SMALLNAME;
	args += APP_MSPROTO;
	if(Config::GetInstance()->cookie.empty() == false)
		args += Config::GetInstance()->cookie;

	me->sendrpl(MSG_IAM, args);
	return 0;
}

/** I've been disconnected from server when I was playing a game. It asks me if I want to rejoin this game.
 *
 * Syntax: REJOIN server:port
 */
int REJOINmsCommand::Exec(PlayerList, EC_Client* me, ParvList parv)
{
	if(!ListServerForm) return 0;

	ListServerForm->Rejoin = parv[1];

	return 0;
}

/** Receive a server name in the list from meta-server
 *
 * Syntax: LSP ip:port nom +/- nbjoueurs nbmax nbgame> maxgames proto
 *             version totusers totgames uptime official
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
 * parv[10]= version
 * parv[11]= totusers
 * parv[12]= totgames
 * parv[13]= uptime
 * parv[14] = official? (1|0)
 */
int LSPmsCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	assert(ListServerForm);

	if(ListServerForm->OnlyOfficials->Checked() && (parv.size() <= 14 || parv[14] != "1"))
		return 0;

	int sock;
	static struct sockaddr_in fsocket_init;
	struct sockaddr_in fsocket = fsocket_init;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::string host = parv[1];

	// Set the timeout
	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 5; // 5seconds timeout
#ifdef WIN32
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
#else
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(stringtok(host, ":").c_str());
	fsocket.sin_port = htons(host.empty() ? SERV_DEFPORT : StrToTyp<uint>(host));

	Timer timer;
	int t0 = SDL_GetTicks();

	/* Connexion */
	if(connect(sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
	{
		Debug(W_DEBUG, "Unable to connect to %s (%d: %s)", parv[2].c_str(), errno, strerror(errno));
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		return 0;
	}

	me->LockScreen();
	if(parv[3][0] == '+' && parv[9] == APP_PVERSION && StrToTyp<int>(parv[4]) > 0)
		ListServerForm->ServerList->AddItem(false,
		                      StringF("%3d  %-23s %2s   %3s/%-3s   %3s/%-3s     %-3s", SDL_GetTicks()-t0, parv[2].substr(0,23).c_str(), parv[9].c_str(),
		                              parv[4].c_str(), parv[5].c_str(), parv[6].c_str(), parv[7].c_str(), parv[8].c_str()),
		                      parv[1], fgreen_color, true, *Font::GetInstance(Font::Normal), parv[2]);
	else
		ListServerForm->ServerList->AddItem(false,
		                      StringF("%4d %-27s %2s    %3s/%-3s    %3s/%-3s       %-3s", SDL_GetTicks()-t0, parv[2].c_str(), parv[9].c_str(),
		                              parv[4].c_str(), parv[5].c_str(), parv[6].c_str(), parv[7].c_str(), parv[8].c_str()),
		                      parv[1], (parv[3][0] == '+' && parv[9] == APP_PVERSION) ? white_color : red_color,
		                      (parv[3][0] == '+' && parv[9] == APP_PVERSION), *Font::GetInstance(Font::Small), parv[2]);

	ListServerForm->nb_chans += StrToTyp<uint>(parv[6]);
	ListServerForm->nb_wchans += StrToTyp<uint>(parv[8]);
	ListServerForm->nb_users += StrToTyp<uint>(parv[4]);
	me->UnlockScreen();
#ifdef WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	if(StrToTyp<int>(parv[9]) > atoi(APP_PVERSION) && parv[10].find("-dev") == std::string::npos)
#ifdef WIN32
		TForm::Message = StringF(_("There is a more recent version of this game. Download it on %s"), APP_SITE);
#else
		TForm::Message = StringF(_("There is a more recent version of this game.\n\n"
		                           "If you have sources, type \"make update && make install\"\n"
		                           "If you have a package, update your distribution or download new version at %s"), APP_SITE);
#endif

	return 0;
}

int EOLmsCommand::Exec(PlayerList players, EC_Client* me, ParvList parv)
{
	assert(ListServerForm);
	ListServerForm->UserStats->SetCaption(StringF(_("There are %d users(s), with %d accounts, %d total connections and %d played games"),
	                                                ListServerForm->nb_users, ListServerForm->nb_tregs, ListServerForm->nb_tusers, ListServerForm->nb_tchans));
	ListServerForm->ChanStats->SetCaption(StringF(_("There are %d game(s) on %d servers, with %d in game and %d in preparation"),
	                                               ListServerForm->nb_chans, ListServerForm->ServerList->Size(), ListServerForm->nb_chans - ListServerForm->nb_wchans,
	                                               ListServerForm->nb_wchans));
	ListServerForm->nb_chans = ListServerForm->nb_wchans = ListServerForm->nb_users = ListServerForm->nb_tchans = ListServerForm->nb_tusers = 0;
	ListServerForm->RefreshButton->SetEnabled();
	ListServerForm->EscarmoucheButton->SetEnabled();
	ListServerForm->MissionButton->SetEnabled();
	ListServerForm->RetourButton->SetEnabled();
	ListServerForm->StatsButton->SetEnabled();
	ListServerForm->RegisterButton->SetEnabled();
	ListServerForm->AccountButton->SetEnabled();

	if(ListServerForm->ServerList->Empty())
		TForm::Message = _("There is no server available. Try later or check your firewall.");

	return 0;
}

/** Statistics about meta-server
 *
 * Syntax: STAT chans users regusers
 */
int STATmsCommand::Exec(PlayerList players, EC_Client* me, ParvList parv)
{
	me->UnsetLogging();
	me->SetConnected();
	assert(ListServerForm);
	ListServerForm->nb_tchans = StrToTyp<uint>(parv[1]);
	ListServerForm->nb_tusers = StrToTyp<uint>(parv[2]);
	if(parv.size() > 3)
		ListServerForm->nb_tregs = StrToTyp<uint>(parv[3]);
	return 0;
}


void TListServerForm::AskForRegister()
{
	TMessageBox msg(_("You can register an account to protect your nickname, and to save your statistics (score, etc.)\n"
	                  "The password will be saved on your computer and you will not have to enter it each times you want to "
	                  "play.\n\n"
	                  "Enter a password:"), BT_OK|BT_CANCEL|HAVE_EDIT, this);
	msg.Edit()->SetPassword();
	if(msg.Show() == BT_OK && msg.Edit()->Text().empty() == false)
	{
		Config::GetInstance()->passwd = msg.Edit()->Text().c_str();
		login = true;
		// On envoie bien le pass en clair au meta-serveur
		MetaServer.Request(MSG_REGNICK, msg.Edit()->Text().c_str());
	}
}

void TListServerForm::AfterDraw()
{
	if(Rejoin.empty() == false)
	{
		if(TMessageBox(StringF(_("You have been disconnected while playing to the %s game. Do you want to rejoin it ?"), Rejoin.c_str()),
				BT_YES|BT_NO, this).Show() == BT_YES)
		{
			MenAreAntsApp::GetInstance()->ConnectedTo(Rejoin, Rejoin);
		}
		Rejoin.clear();
	}
	if(login && Config::GetInstance()->passwd.empty())
	{
		TMessageBox mb(_("This nickname is registered.\n\nEnter the password:"), BT_OK|BT_CANCEL|HAVE_EDIT, this);
		mb.Edit()->SetPassword();
		if(mb.Show() == BT_OK && mb.Edit()->Text().empty() == false)
		{
			Config::GetInstance()->passwd = mb.Edit()->Text().c_str();
			MetaServer.sendrpl(MSG_LOGIN, mb.Edit()->Text().c_str());
		}
		else
		{
			login = false;
			want_quit = true;
		}
	}
	if(MenAreAntsApp::GetInstance()->IsFirstRun() && !login && ServerList->Empty() == false)
	{
		if(TMessageBox(_("Hello! This is your first run!\nYou can register an account to have score saving and your nickname protected.\n\n"
		                 "Do you want to register it?"), BT_YES|BT_NO, this).Show() == BT_YES)
			AskForRegister();
		MenAreAntsApp::GetInstance()->FirstRunDone();
	}
	if(timer.time_elapsed() > 60)
	{
		MenAreAntsApp::GetInstance()->RefreshList();
		timer.reset();
	}
	if(!MetaServer.IsConnected() && MetaServer.IsLogging())
	{
		do
		{
			SDL_Event event;
			SDL_PollEvent( &event);
			TMessageBox(_("Please wait while connecting..."), 0, this).Draw();
			SDL_Delay(20);
		} while(!MetaServer.IsConnected() && MetaServer.IsLogging() && !login);
	}
}

void TListServerForm::OnClic(const Point2i& mouse, int button, bool& stop)
{
	//if(!MetaServer.IsConnected()) return;
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
		MenAreAntsApp::GetInstance()->ConnectedTo(ServerList->SelectedItem()->Name(), ServerList->SelectedItem()->Value());
		MenAreAntsApp::GetInstance()->RefreshList();
	}
	else if(ConnectToButton->Test(mouse, button))
	{
		TMessageBox msg(_("Please enter a server hostname and a port, in form hostname[:port]"), BT_OK|BT_CANCEL|HAVE_EDIT, this);
		if(msg.Show() == BT_OK)
		{
			MenAreAntsApp::GetInstance()->ConnectedTo(msg.Edit()->Text(), msg.Edit()->Text());
			MenAreAntsApp::GetInstance()->RefreshList();
		}
	}
	else if(AccountButton->Test(mouse, button))
		TMessageBox(_("This button does nothing..."), BT_OK, this).Show();
	else if(RegisterButton->Test(mouse, button))
	{
		AskForRegister();
	}
	else if(RetourButton->Test(mouse, button))
	{
		MetaServer.SetWantDisconnect();
		want_quit = true;
	}
	else if(MissionButton->Test(mouse, button) || EscarmoucheButton->Test(mouse, button))
	{
		int size = ServerList->Size();
		if(!size)
		{
			TMessageBox(_("There isn't any disponible server\n\nPlease retry later."),
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
			TMessageBox(_("There isn't any server to host your game.\n\nPlease retry later."),
						BT_OK, this).Show();
		else
		{
			EC_Client* client = MenAreAntsApp::GetInstance()->Connect(ServerList->ReadValue(r));
			if(!client)
				return;
			if(!MenAreAntsApp::GetInstance()->GameInfos(NULL, ConnectedForm, MissionButton->Test(mouse, button) ? G_MISSION : G_ESCARMOUCHE))
				TMessageBox(_("Unable to create this game.\n There are too games on this server. Please retry."),
				            BT_OK, ConnectedForm).Show();
			if(!client->IsConnected())
				TMessageBox(_("You have been disconnected"), BT_OK).Show();
			client->SetWantDisconnect();
			MenAreAntsApp::GetInstance()->RefreshList();
		}
	}
}

void TListServerForm::SetOnlyOfficials(TObject* obj, void* forminst)
{
	TCheckBox* CheckBox = dynamic_cast<TCheckBox*>(obj);
	TListServerForm* form = static_cast<TListServerForm*>(forminst);
	if(!CheckBox || !form) return;

	MenAreAntsApp::GetInstance()->RefreshList();
}

TListServerForm::TListServerForm(ECImage* w)
	: TForm(w), nb_chans(0), nb_wchans(0), nb_users(0), nb_tchans(0), nb_tusers(0), nb_tregs(0), login(false)
{
	ServerList = AddComponent(new TListBox(Rectanglei(0, 0, 500,350)));
	ServerList->SetXY(Window()->GetWidth()/2 - ServerList->Width()/2 - 50, Window()->GetHeight()/2 - ServerList->Height()/2 + 70);
	ServerList->SetGrayDisable(false);

	Welcome = AddComponent(new TLabel(30,StringF(_("Welcome to Men Are Ants, %s!"), Config::GetInstance()->nick.c_str()),
	                                  white_color, Font::GetInstance(Width() <= 800 ? Font::Large : Font::Huge)));

	Label1 = AddComponent(new TLabel(ServerList->Y()-50, _("Please select a server in this list:"), white_color, Font::GetInstance(Font::Big)));

	Label2 = AddComponent(new TLabel(ServerList->X(), ServerList->Y()-20, "", white_color, Font::GetInstance(Font::Normal)));
	Label2->SetCaption(_("Ping  Name                 Proto   Players   Games  On Standby"));

	MissionButton = AddComponent(new TButton(0, 0, 220,50));
	MissionButton->SetHint(_("Mission"));
	MissionButton->SetImage(new ECSprite(Resources::MissionButton(), Video::GetInstance()->Window()));
	EscarmoucheButton = AddComponent(new TButton(0, 0, 220,50));
	EscarmoucheButton->SetHint(_("Skirmich against computer"));
	EscarmoucheButton->SetImage(new ECSprite(Resources::EscarmoucheButton(), Video::GetInstance()->Window()));

	int bottom = Label1->Y();
	if (Label1->Y() - (Welcome->Y() + Welcome->Height()) < MissionButton->Height())
	{
		Label1->Hide();
		bottom = ServerList->Y();
	}

	int f = bottom/2 + (Welcome->Y() + Welcome->Height())/2;
	if (bottom - (Welcome->Y() + Welcome->Height()) >= MissionButton->Height() + EscarmoucheButton->Height())
	{
		MissionButton->SetXY(Window()->GetWidth()/3, f - MissionButton->Height());
		MissionLabel = AddComponent(new TLabel(MissionButton->X() + MissionButton->Width() + 10,
		                                       MissionButton->Y() + MissionButton->Height()/2 - Font::GetInstance(Font::Big)->GetHeight()/2,
		                                       _("Mission"), white_color, Font::GetInstance(Font::Big)));
		EscarmoucheButton->SetXY(Window()->GetWidth()/3, f);
		EscarmoucheLabel = AddComponent(new TLabel(EscarmoucheButton->X() + EscarmoucheButton->Width() + 10,
		                                           EscarmoucheButton->Y() + EscarmoucheButton->Height()/2 - Font::GetInstance(Font::Big)->GetHeight()/2,
		                                           _("Skirmich"), white_color, Font::GetInstance(Font::Big)));
	}
	else
	{
		MissionButton->SetXY(ServerList->X() + 75, f-MissionButton->Height()/2);
		MissionLabel = NULL;
		EscarmoucheButton->SetXY(ServerList->X() + ServerList->Width() - EscarmoucheButton->Width() - 75, f-EscarmoucheButton->Height()/2);
		EscarmoucheLabel = NULL;
	}

	UserStats = AddComponent(new TLabel(ServerList->X()-25, ServerList->Y()+ServerList->Height()+10, " ", white_color, Font::GetInstance(Font::Normal)));
	ChanStats = AddComponent(new TLabel(ServerList->X()-25, UserStats->Y()+UserStats->Height(), " ", white_color, Font::GetInstance(Font::Normal)));

	int button_x = ServerList->X() + ServerList->Width();

	ConnectButton = AddComponent(new TButton(button_x, ServerList->Y(), 150,50));
	ConnectButton->SetImage(new ECSprite(Resources::OkButton(), Video::GetInstance()->Window()));
	ConnectButton->SetHint(_("Join selected server"));
	ConnectButton->SetEnabled(false);

	ConnectToButton = AddComponent(new TButton(ConnectButton->X()+ConnectButton->Width(), ConnectButton->Y(), 150,50));
	ConnectToButton->SetImage(new ECSprite(Resources::HostButton(), Video::GetInstance()->Window()));
	ConnectToButton->SetHint(_("Connect to a specific server"));

	RegisterButton = AddComponent(new TButton(button_x, ConnectButton->Y()+ConnectButton->Height(), 150,50));
	RegisterButton->SetImage(new ECSprite(Resources::SaveButton(), Video::GetInstance()->Window()));
	RegisterButton->SetHint(_("Register your nickname to save your stats and to protect it from other users"));

	AccountButton = AddComponent(new TButton(button_x, ConnectButton->Y()+ConnectButton->Height(), 150,50));
	AccountButton->SetImage(new ECSprite(Resources::AccountButton(), Video::GetInstance()->Window()));
	AccountButton->SetHint(_("See your profile"));
	AccountButton->Hide();

	StatsButton = AddComponent(new TButton(RegisterButton->X()+RegisterButton->Width(), ConnectButton->Y()+ConnectButton->Height(), 150,50));
	StatsButton->SetHint(_("Show scoring"));
	StatsButton->SetImage(new ECSprite(Resources::ScoresButton(), Video::GetInstance()->Window()));
	StatsButton->SetOnClick(TGlobalScoresForm::Scores, 0);

	RefreshButton = AddComponent(new TButton(button_x, RegisterButton->Y()+RegisterButton->Height(), 150,50));
	RefreshButton->SetImage(new ECSprite(Resources::RefreshButton(), Video::GetInstance()->Window()));
	RefreshButton->SetHint(_("Refresh server list"));

	RetourButton = AddComponent(new TButton(RefreshButton->X()+RefreshButton->Width(),RegisterButton->Y()+RegisterButton->Height(),150,50));
	RetourButton->SetImage(new ECSprite(Resources::BackButton(), Video::GetInstance()->Window()));

	OnlyOfficials = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), button_x, ServerList->Y()+ServerList->Height()-20, _("Show only official servers"), white_color));
	OnlyOfficials->SetOnClick(TListServerForm::SetOnlyOfficials, this);
	OnlyOfficials->Check();

	SetBackground(Resources::Titlescreen());
}

EC_Client* MenAreAntsApp::Connect(std::string host)
{
	EC_Client* client = &Server;

	client->ClearCommands();
	/* Ajout des commandes            CMDNAME		FLAGS	ARGS */
	client->AddCommand(new ARMCommand(MSG_ARM,		0,	0));

	client->AddCommand(new PIGCommand(MSG_PING,		0,	0));
	client->AddCommand(new HELCommand(MSG_HELLO,		0,	1));
	client->AddCommand(new AIMCommand(MSG_LOGGED,		0,	1));
	client->AddCommand(new ERRORCommand(MSG_ERROR,		0,	1));
	client->AddCommand(new MAJCommand(MSG_MAJ,		0,	1));
	client->AddCommand(new MOTDCommand(MSG_MOTD,		0,	0));
	client->AddCommand(new EOMCommand(MSG_ENDOFMOTD,	0,	0));
	client->AddCommand(new STATCommand(MSG_STAT,		0,	7));

	client->AddCommand(new LSPCommand(MSG_GLIST,		0,	3));
	client->AddCommand(new EOLCommand(MSG_ENDOFGLIST,	0,	0));

	client->AddCommand(new BPCommand(MSG_BREAKPOINT,	0,	1));

	client->AddCommand(new JOICommand(MSG_JOIN,		0,	1));
	client->AddCommand(new SETCommand(MSG_SET,		0,	1));
	client->AddCommand(new PLSCommand(MSG_PLIST,		0,	1));
	client->AddCommand(new LEACommand(MSG_LEAVE,		0,	0));
	client->AddCommand(new KICKCommand(MSG_KICK,		0,	1));
	client->AddCommand(new MSGCommand(MSG_MSG,		0,	1));
	client->AddCommand(new AMSGCommand(MSG_AMSG,		0,	1));
	client->AddCommand(new INFOCommand(MSG_INFO,		0,	1));

	client->AddCommand(new LSMCommand(MSG_LISTMAPS,		0,	3));
	client->AddCommand(new EOMAPCommand(MSG_ENDOFMAPS,	0,	0));
	client->AddCommand(new SMAPCommand(MSG_SENDMAP,		0,	1));
	client->AddCommand(new EOSMAPCommand(MSG_ENDOFSMAP,	0,	0));

	client->AddCommand(new SCOCommand(MSG_SCORE,		0,	4));
	client->AddCommand(new REJOINCommand(MSG_REJOIN,	0,	1));
	client->AddCommand(new ADMINCommand(MSG_ADMIN,		0,	0));

	std::string hostname = stringtok(host, ":");
	Server.Connect(hostname.c_str(), host.empty() ? SERV_DEFPORT : StrToTyp<uint>(host));

	WAIT_EVENT_T(client->IsConnected() || client->Error(), i, 5);

	if(!client->IsConnected())
	{
		std::string msg;

		client->SetWantDisconnect();

		msg = _("Unable to connect:\n\n") + client->CantConnect();
		TMessageBox(msg, BT_OK, ListServerForm).Show();

		return 0;
	}

	return client;
}

void MenAreAntsApp::ConnectedTo(std::string name, std::string host)
{
	if(Server.IsConnected())
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
		ConnectedForm->Welcome->SetCaption(StringF(_("Welcome to %s, %s !"), name.c_str(), client->GetNick().c_str()));

		ConnectedForm->Run();
	}
	catch(const TECExcept &e)
	{
		vDebug(W_ERR|W_SEND, e.Message(), e.Vars());
		Server.SetWantDisconnect();
		TMessageBox mb(_("An error was occured in game!!\n\n"
		                 "It has been send to MenAreAnts' coders, and they are going to fix it.\n\n"
		                 "Sorry for obstruct caused"), BT_OK, NULL);
				mb.SetBackGround(Resources::Titlescreen());
				mb.Show();
#ifdef DEBUG
		throw;
#endif
	}
	MyFree(ConnectedForm);

	return;


}

/********************************************************************************************
 *                               TConnectedForm                                             *
 ********************************************************************************************/

/** We are connected to server.
 *
 * Syntax: HEL prog version
 */
int HELCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	ECArgs args = Config::GetInstance()->nick;
	args += CLIENT_SMALLNAME;
	args += APP_PVERSION;
	if(Config::GetInstance()->cookie.empty() == false)
		args += Config::GetInstance()->cookie;

	me->sendrpl(MSG_IAM, args);
	return 0;
}

/** Server acknowledges my nickname.
 *
 * Syntax: AIM nick [cookie]
 */
int AIMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->set_nick(parv[1]);
	me->UnsetLogging();
	me->SetConnected();
	if(me == &MetaServer && ListServerForm)
	{
		if(parv.size() > 2)
			Config::GetInstance()->cookie = parv[2];
		Config::GetInstance()->nick = parv[1];
		ListServerForm->Welcome->SetCaption(StringF(_("Welcome to Men Are Ants, %s!"), me->GetNick().c_str()));

		if(me->Request().empty() == false)
		{
			me->sendbuf(me->Request());
			me->Request().clear();
			me->sendrpl(MSG_BYE);
		}
	}
	return 0;
}

/** Received a PING from server.
 *
 * Syntax: PIG
 */
int PIGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->sendrpl(MSG_PONG);

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
			me->SetCantConnect(_("This game isn't compatible with this server!?"));
			break;
		case '+':
			me->SetCantConnect(_("You use a version more recent than that of the server"));
			break;
		case '-':
			me->SetCantConnect(std::string(_("You have to update Men Are Ants to play on this server.\n\n")) +
#ifndef WIN32
			                     _("Type \"make update && sudo make install\".\n")
#else
			                     _("Download MenAreAnts.zip on website.\n")
#endif
			                     + StringF(_("Go on %s to get more informations."), APP_SITE));
			break;
		default:
			me->SetCantConnect(_("Unable to connect to this server."));
			vDebug(W_DESYNCH|W_SEND, "Reception d'un MAJ bizarre !",
		                                VName(parv[1]));
			break;
	}
	return 0;
}

/** It's a notification to my admin status.
 *
 * Syntax: ADMIN
 */
int ADMINCommand::Exec(PlayerList, EC_Client* me, ParvList parv)
{
	if(ConnectedForm)
	{
		ConnectedForm->RehashButton->SetVisible(true);
		ConnectedForm->KillButton->SetVisible(true);
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

	ConnectedForm->Motd->AddItem(parv.size() > 1 ? parv[1] : "", white_color);
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
 * Syntax: STAT nbactco nbco nbch chinwait chingame chtot uptime version
 */
int STATCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(ConnectedForm)
	{
		ConnectedForm->Uptime->SetCaption(StringF(_("This server is running from %s"), duration(time(NULL) - StrToTyp<time_t>(parv[7]))));
		ConnectedForm->UserStats->SetCaption(StringF(_("There are %s users, with %s total connections and %s played games"),
		                                               parv[1].c_str(), parv[2].c_str(), parv[6].c_str()));
		ConnectedForm->ChanStats->SetCaption(StringF(_("There are %s game(s), with %s in game and %s in preparation."),
		                                               parv[3].c_str(), parv[5].c_str(), parv[4].c_str()));
		if(parv.size() > 8)
			ConnectedForm->ServerStats->SetCaption(parv[8]);
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
	bool enabled = (parv.size() <= 4 || StrToTyp<int>(parv[3]) < StrToTyp<int>(parv[4]));
	if(parv[2][0] == '+')
	{
		if(parv[4] == "0")
			ConnectedForm->GList->AddItem(false, StringF("%-8s %2s", parv[1].c_str(), parv[3].c_str()), parv[1],
			                              white_color, true);
		else if(parv.size() > 5)
			ConnectedForm->GList->AddItem(false, StringF("%-8s %2s/%-2s %s", parv[1].c_str(), parv[3].c_str(),
			                                                                 parv[4].c_str(), parv[5].c_str()), parv[1],
			                              enabled ? white_color : red_color,
			                              enabled ? true : false);
		else
			ConnectedForm->GList->AddItem(false, StringF("%-8s %2s/%-2s", parv[1].c_str(), parv[3].c_str(),
			                                                              parv[4].c_str()), parv[1],
			                              enabled ? white_color : red_color,
			                              enabled ? true : false);
	}
	else
		ConnectedForm->GList->AddItem(false, StringF(_("%-8s  Playing: %s"), parv[1].c_str(), parv[5].c_str()), parv[1],
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

void TConnectedForm::AfterDraw()
{
	if(Rejoin.empty() == false)
	{
		MenAreAntsApp::GetInstance()->RecoverGame(Rejoin);
		Rejoin.clear();
	}
	if(!client->IsConnected())
	{
		if(!client->WantDisconnect())
			TMessageBox(_("You have been disconnected."), BT_OK, this).Show();

		want_quit = true;
		return;
	}
	if(timer.time_elapsed(true) > 60 || refresh)
	{
		JoinButton->SetEnabled(false);
		GList->ClearItems();
		EOL = false;
		client->sendrpl(MSG_GLIST);
		WAIT_EVENT(EOL, j);
		if(!EOL)
		{
			TMessageBox(_("Server is too slow. Disconnecting..."), BT_OK, this).Show();
			want_quit = true;
			return;
		}

		refresh = false;
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
		MenAreAntsApp::GetInstance()->GameInfos(GList->SelectedItem()->Value().c_str(), this);
		refresh = true;
		timer.reset();
		client->sendrpl(MSG_STAT);
	}
	else if(RefreshButton->Test(mouse, button))
		refresh = true;
	else if(CreerButton->Test(mouse, button))
	{
		MenAreAntsApp::GetInstance()->GameInfos(NULL, this);
		refresh = true;
		timer.reset();
		client->sendrpl(MSG_STAT);
	}
	else if(mouse.x < 10 && mouse.y < 10)
	{
		TMessageBox mb(_("You are in an administration login proccess.\n\n"
		                 "Enter the password:"), HAVE_EDIT|BT_OK|BT_CANCEL, this);
		if(mb.Show() == BT_OK)
			client->sendrpl(MSG_ADMIN, ECArgs("LOGIN", mb.Edit()->Text()));
	}
	else if(RehashButton->Test(mouse, button))
	{
		client->sendrpl(MSG_ADMIN, "REHASH");
	}
	else if(KillButton->Test(mouse, button))
	{
		TMessageBox mb(_("Enter user's nickname you want to kill:"), HAVE_EDIT|BT_OK|BT_CANCEL, this);
		if(mb.Show() == BT_OK)
			client->sendrpl(MSG_ADMIN, ECArgs("KILL", mb.Edit()->Text()));
	}
}

TConnectedForm::TConnectedForm(ECImage* w)
	: TForm(w), refresh(true), client(0)
{
	Motd = AddComponent(new TMemo(Font::GetInstance(Font::Small), 0, 0, 400,350, 0));
	Motd->SetXY(Window()->GetWidth()/2 - Motd->Width()/2 - (200+150+10+10)/2, Window()->GetHeight()/2 - Motd->Height()/2);

	MOTDLabel = AddComponent(new TLabel(Motd->X()+5, Motd->Y()-20, _("Message of the day"), white_color, Font::GetInstance(Font::Normal)));

	GList = AddComponent(new TListBox(Rectanglei(300,350,200,350)));
	GList->SetXY(Motd->X()+Motd->Width()+10, Motd->Y());
	GList->SetGrayDisable(false);

	ListLabel = AddComponent(new TLabel(GList->X()+5, GList->Y()-20, _("Game list:"), white_color, Font::GetInstance(Font::Normal)));

	Welcome = AddComponent(new TLabel(Motd->Y()/2,"You are connected", white_color, Font::GetInstance(Width() < 1000 ? Font::Big : Font::Large)));

	int button_x = GList->X() + GList->Width();

	JoinButton = AddComponent(new TButton(button_x, GList->Y(), 150,50));
	JoinButton->SetEnabled(false);
	JoinButton->SetImage(new ECSprite(Resources::JoinButton(), Video::GetInstance()->Window()));
	JoinButton->SetHint(_("Join a game"));
	CreerButton = AddComponent(new TButton(button_x+JoinButton->Width(),GList->Y(), 150,50));
	CreerButton->SetImage(new ECSprite(Resources::CreateButton(), Video::GetInstance()->Window()));
	CreerButton->SetHint(_("Create a game"));
	RefreshButton = AddComponent(new TButton(button_x,JoinButton->Y()+JoinButton->Height(),150,50));
	RefreshButton->SetImage(new ECSprite(Resources::RefreshButton(), Video::GetInstance()->Window()));
	RefreshButton->SetHint(_("Refresh list"));
	DisconnectButton = AddComponent(new TButton(button_x+RefreshButton->Width(),JoinButton->Y()+JoinButton->Height(),150,50));
	DisconnectButton->SetImage(new ECSprite(Resources::BackButton(), Video::GetInstance()->Window()));
	DisconnectButton->SetHint(_("Disconnect from server"));
	RehashButton = AddComponent(new TButton(button_x,DisconnectButton->Y()+DisconnectButton->Height(),150,50));
	RehashButton->SetVisible(false);
	RehashButton->SetImage(new ECSprite(Resources::RefreshConfButton(), Video::GetInstance()->Window()));
	RehashButton->SetHint(_("Rehash configuration"));
	KillButton = AddComponent(new TButton(button_x+RehashButton->Width(),DisconnectButton->Y()+DisconnectButton->Height(),150,50));
	KillButton->SetVisible(false);
	KillButton->SetImage(new ECSprite(Resources::KillButton(), Video::GetInstance()->Window()));
	KillButton->SetHint(_("Kill a player"));

	Uptime =    AddComponent(new TLabel(75,Window()->GetHeight()-90," ", white_color, Font::GetInstance(Font::Normal)));
	UserStats = AddComponent(new TLabel(75,Uptime->Y()+Uptime->Height()," ", white_color, Font::GetInstance(Font::Normal)));
	ChanStats = AddComponent(new TLabel(75,UserStats->Y()+UserStats->Height()," ", white_color,
	                                    Font::GetInstance(Font::Normal)));
	ServerStats = AddComponent(new TLabel(75,ChanStats->Y()+ChanStats->Height()," ", white_color,
	                                    Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TGlobalScoresForm                                          *
 ********************************************************************************************/

// SCORE <nick> <deaths> <killed> <creations> <scores> <best_revenu> <nbgames> <victories> <regtime> <lastvisit>
int SCOREmsCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GlobalScoresForm)
	{
		if(GlobalScoresForm->ListBox->Empty())
			GlobalScoresForm->BestPlayer->SetCaption(StringF(_("And the better player is.... %s with %s points!"),
			                                                   parv[1].c_str(), parv[5].c_str()));

		int defeats = StrToTyp<int>(parv[7]) - StrToTyp<int>(parv[8]);
		GlobalScoresForm->ListBox->AddItem(false,
		                      StringF("%-15s%10s %11s %9d   %-11s %9s", parv[1].c_str(), parv[5].c_str(), parv[8].c_str(),
		                                                                defeats, parv[3].c_str(), StringF(_("$%s"), parv[6].c_str()).c_str()),
		                      parv[1]);

		if(GlobalScoresForm->best_incomes < StrToTyp<int>(parv[6]))
		{
			GlobalScoresForm->best_incomes = StrToTyp<int>(parv[6]);
			GlobalScoresForm->BestIncomes->SetCaption(StringF(_("The one who incomes the must of money is %s with $%s per turn!"),
			                                                   parv[1].c_str(), parv[6].c_str()));
		}
		if(GlobalScoresForm->best_kills < StrToTyp<int>(parv[3]))
		{
			GlobalScoresForm->best_kills = StrToTyp<int>(parv[3]);
			GlobalScoresForm->BestKills->SetCaption(StringF(_("%s has killed %s men! He's a butcher!"),
			                                                  parv[1].c_str(), parv[3].c_str()));
		}
	}
	return 0;
}

void TGlobalScoresForm::Scores(TObject*, void*)
{
	GlobalScoresForm = new TGlobalScoresForm(Video::GetInstance()->Window());

	MetaServer.Request(MSG_SCORE);

	GlobalScoresForm->Run();

	MyFree(GlobalScoresForm);
}

void TGlobalScoresForm::OnClic(const Point2i& mouse, int button, bool& stop)
{
	if(RetourButton->Test(mouse, button))
		want_quit = true;
}

TGlobalScoresForm::TGlobalScoresForm(ECImage* im)
	: TForm(im), best_incomes(0), best_kills(0)
{
	ListBox = AddComponent(new TListBox(Rectanglei(0, 0, 520,350)));
	ListBox->SetXY(Window()->GetWidth()/2 - ListBox->Width()/2 - 50, Window()->GetHeight()/2 - ListBox->Height()/2 + 70);
	ListBox->SetGrayDisable(false);

	Headers = AddComponent(new TLabel(ListBox->X(), ListBox->Y()-20, "", white_color, Font::GetInstance(Font::Normal)));
	Headers->SetCaption(_("Nickname         Score  Victories  Defeats  Killed  Best incomes"));

	Title = AddComponent(new TLabel(10, _("Scores"), white_color, Font::GetInstance(Font::Huge)));

	BestPlayer = AddComponent(new TLabel(50, " ", white_color, Font::GetInstance(Font::Big)));

	BestIncomes = AddComponent(new TLabel(BestPlayer->Y()+BestPlayer->Height()+15, " ", white_color, Font::GetInstance(Font::Normal)));

	BestKills = AddComponent(new TLabel(BestIncomes->Y()+BestIncomes->Height()+15, " ", white_color, Font::GetInstance(Font::Normal)));

	int button_x = ListBox->X() + ListBox->Width() + 10;

	RetourButton = AddComponent(new TButton(button_x,ListBox->Y(),150,50));
	RetourButton->SetImage(new ECSprite(Resources::BackButton(), Video::GetInstance()->Window()));

	SetBackground(Resources::Titlescreen());
}
