/* src/main.cpp - Main file
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

#include <SDL.h>
#include <SDL_image.h>
#include <cerrno>
#include <signal.h>

#ifndef WIN32
#include <sys/stat.h>
#include <dirent.h>
#include <sys/resource.h>
#else
#include <winsock2.h>
#endif

#include "Main.h"
#include "Config.h"
#include "Debug.h"
#include "Defines.h"
#include "Resources.h"
#include "Sockets.h"
#include "Sound.h"
#include "Version.h"
#include "gui/Boutton.h"
#include "gui/BouttonText.h"
#include "gui/ComboBox.h"
#include "gui/Cursor.h"
#include "gui/Form.h"
#include "gui/Fps.h"
#include "gui/Image.h"
#include "gui/Label.h"
#include "gui/Memo.h"
#include "gui/MessageBox.h"
#include "tools/Font.h"
#include "tools/Images.h"
#include "tools/Video.h"
#include <functional>

#if !defined(DEBUG) && defined(UNSTABLE)
#error Vous tentez de compiler une version instable. Si vous Ãªtes certain de vouloir la compiler, rajoutez --enable-debug dans les options du script ./configure
#endif

#ifdef WIN32
#define PACKAGE "menareants"
#endif

#define GETTEXT_DOMAIN PACKAGE

void I18N_SetDir(const std::string &dir){
  bindtextdomain(GETTEXT_DOMAIN, dir.c_str());
  bind_textdomain_codeset (GETTEXT_DOMAIN, "UTF-8");
}

void InitI18N(const std::string &dir){
  setlocale (LC_ALL, "");
  I18N_SetDir (dir);
  textdomain(GETTEXT_DOMAIN);
}

class TMainForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TMainForm(ECImage*);

/* Composants */
public:

	TButton*    PlayButton;
	TButton*    OptionsButton;
	TButton*    CreditsButton;
	TButton*    QuitterButton;
	TButton*    MapEditorButton;
	TImage*     Title;
	TLabel*     Version;
	TFPS*       FPS;

	static bool enter_in_main;
	void SetRelativePositions();
};

bool TMainForm::enter_in_main = true;

MenAreAntsApp* MenAreAntsApp::singleton = NULL;
MenAreAntsApp* MenAreAntsApp::GetInstance()
{
	if (singleton == NULL)
		singleton = new MenAreAntsApp();

	return singleton;
}

char* MenAreAntsApp::get_title()
{
	return APP_NAME;
}

void MenAreAntsApp::WantQuit(TObject*, void*)
{
	MenAreAntsApp::GetInstance()->want_quit = true;
}

void MenAreAntsApp::WantPlay(TObject*, void*)
{
	MenAreAntsApp::GetInstance()->ServerList();
	TMainForm::enter_in_main = true;
}

void MenAreAntsApp::WantMapEditor(TObject*, void*)
{
	MenAreAntsApp::GetInstance()->MapEditor();
	TMainForm::enter_in_main = true;
}

void MenAreAntsApp::WantConfig(TObject*, void* b)
{
	Config::GetInstance()->Configuration((bool)b);
	TMainForm::enter_in_main = true;
}

void MenAreAntsApp::UnLoad()
{
	EC_Client::Exit();
	Config::GetInstance()->save();
#ifdef WIN32
	WSACleanup();
#endif
	Resources::Unload();
	TTF_Quit();
	Sound::End();
	delete Video::GetInstance();
	delete Config::GetInstance();
}

int MenAreAntsApp::main(int argc, char **argv)
{
	try
	{
#ifndef WIN32
		signal(SIGPIPE, SIG_IGN);

		struct rlimit rlim; /* used for core size */
		if(!getrlimit(RLIMIT_CORE, &rlim) && rlim.rlim_cur != RLIM_INFINITY)
		{
			rlim.rlim_cur = RLIM_INFINITY;
			rlim.rlim_max = RLIM_INFINITY;
			setrlimit(RLIMIT_CORE, &rlim);
		}

#endif

		srand( (long)time(NULL) );

		InitI18N(INSTALL_LOCALEDIR);

		Config* conf = Config::GetInstance();
#ifndef WIN32
		path = GetHome();
		path += "/.menareants/";
		DIR *d;
		if(!(d = opendir(path.c_str())))
			mkdir( path.c_str(), 0755 );
		else closedir(d);

		conf->SetFileName( path + CONFIG_FILE );
#else
		conf->SetFileName(CONFIG_FILE);
#endif
		conf->load();

		Video* video = Video::GetInstance();
		video->InitWindow();

		video->SetWindowIcon(PKGDATADIR_PICS "MAA.xpm" );

		ECImage *loading_image = Resources::Loadscreen();

		if(loading_image->GetWidth() != video->Width() ||
		   loading_image->GetHeight() != video->Height())
			loading_image->Zoom(double(video->Width()) / (double)loading_image->GetWidth(),
	                            double(video->Height()) / (double)loading_image->GetHeight(),
	                            true);

		loading_image->Draw();

		Cursor.SetWindow(video->Window());
		Cursor.Init();

		EC_Client::Init();
		mutex = EC_Client::Mutex();

		if (TTF_Init()==-1) {
			std::cerr << "TTF_Init: "<< TTF_GetError() << std::endl;
			return false;
		}

		Sound::Init();

#ifdef WIN32
		WSADATA WSAData;
		if(WSAStartup(MAKEWORD(2,0), &WSAData) != 0)
			throw ECExcept(VIName(WSAGetLastError()), "Unable to initialize windows sockets");
#endif

		SDL_UpdateRect(video->Window()->Img, 0, 0, 0, 0);
		video->Flip();

		if(first_run)
			MenAreAntsApp::WantConfig(0,(void*)true);

		TMainForm* MainForm = new TMainForm(video->Window());
		MainForm->PlayButton->SetOnClick(MenAreAntsApp::WantPlay, this);
		MainForm->QuitterButton->SetOnClick(MenAreAntsApp::WantQuit, this);
		MainForm->CreditsButton->SetOnClick(MenAreAntsApp::WantCredits, this);
		MainForm->MapEditorButton->SetOnClick(MenAreAntsApp::WantMapEditor, this);
		MainForm->OptionsButton->SetOnClick(MenAreAntsApp::WantConfig, (void*)false);
		Sound::SetMusicList(MENU_MUSIC);
#ifdef WIN32
		TMessageBox(_("You are using Microsoft Windows. Please note that the Windows version of this game can be bugged.\n\n"
		              "This is a free software, please use a GOOD Operating System, like GNU/Linux."), BT_OK, MainForm).Show();
#endif
		do
		{
			MainForm->Actions();
			if(TMainForm::enter_in_main)
			{
				TMainForm::enter_in_main = false;
#ifdef BUGUED_INTRO
				ECSprite sprite(Resources::Intro(), video->Window());
				if(sprite.GetWidth() != video->Width() ||
				   sprite.GetHeight() != video->Height())
					sprite.Zoom(double(video->Width()) / (double)sprite.GetWidth(),
	                             double(video->Height()) / (double)sprite.GetHeight(),
	                             true);
				sprite.SetRepeat(false);
				sprite.set(0,0);
				while(sprite.Anim())
				{
					sprite.draw();
					video->Window()->Flip();
				}
#endif
				MainForm->SetRelativePositions();
				MainForm->SetMustRedraw();
			}
			MainForm->Update();
		} while(!want_quit);

		MyFree(MainForm);
	}
	catch (const TECExcept &e)
	{
		std::cout << "Received an ECExcept error: " << std::endl;
		std::cout << e.Message() << std::endl;
#ifdef DEBUG
		std::cout << e.Vars() << std::endl;
#endif
	}
#ifndef DEBUG
	catch (const std::exception &err)
	{
		std::cout << std::endl << "Exception caught from STL:" << std::endl;
		std::cout << err.what() << std::endl;
	}
#endif

	UnLoad();
	return 0;

}

int main (int argc, char **argv)
{
  MenAreAntsApp::GetInstance()->main(argc,argv);
  delete MenAreAntsApp::GetInstance();
  exit (EXIT_SUCCESS);
}

/********************************************************************************************
 *                                    TMainForm                                             *
 ********************************************************************************************/

void TMainForm::SetRelativePositions()
{
	if(Height() < 800)
		Title->SetImage(Resources::TitleMini(), false);
	else
		Title->SetImage(Resources::Title(), false);

	Title->SetXY(Window()->GetWidth()/2 - Title->Width()/2, 50);
	int left = Width()/2 - (PlayButton->Width() + 40 + OptionsButton->Width() + 40 + MapEditorButton->Width()) / 2;
	int up = Height()/2 - 40;
	if(up+198+QuitterButton->Height() > Height()) up = Height() - QuitterButton->Height() - 198;

	PlayButton->SetXY(left, up);
	OptionsButton->SetXY(PlayButton->X()+PlayButton->Width()+40, up);
	MapEditorButton->SetXY(OptionsButton->X()+OptionsButton->Width()+40, up);

	up += 198;
	left += 112;
	CreditsButton->SetXY(left, up);
	QuitterButton->SetXY(CreditsButton->X() + CreditsButton->Width() + 40, up);
	Version->SetXY(Title->X() + Title->Width() - Version->Width(), Title->Y()+Title->Height() + ((Height() < 800) ? 5 : 20));
}

TMainForm::TMainForm(ECImage* w)
	: TForm(w)
{
	CreditsButton = AddComponent(new TButton(300,390, 150,50));
	CreditsButton->SetImage(new ECSprite(Resources::CreditsButton(), Video::GetInstance()->Window()));
	CreditsButton->SetHint(_("Credits"));

	QuitterButton = AddComponent(new TButton(300,470, 150,50));
	QuitterButton->SetImage(new ECSprite(Resources::QuitButton(), Video::GetInstance()->Window()));
	QuitterButton->SetHint(_("Exit"));

	PlayButton = AddComponent(new TButton(300,150, 150,50));
	PlayButton->SetImage(new ECSprite(Resources::PlayButton(), Video::GetInstance()->Window()));
	PlayButton->SetHint(_("Play a game"));

	OptionsButton = AddComponent(new TButton(300,230, 150,50));
	OptionsButton->SetImage(new ECSprite(Resources::OptionsButton(), Video::GetInstance()->Window()));
	OptionsButton->SetHint(_("Options"));

	MapEditorButton = AddComponent(new TButton(300,310, 150,50));
	MapEditorButton->SetImage(new ECSprite(Resources::MapEditorButton(), Video::GetInstance()->Window()));
	MapEditorButton->SetHint(_("Map Editor"));

	Title = AddComponent(new TImage(300, 100, Resources::Title(), false));

	Version = AddComponent(new TLabel(Window()->GetWidth()-50,105,APP_VERSION, white_color, Font::GetInstance(Font::Big)));

	FPS = AddComponent(new TFPS(5, 5, Font::GetInstance(Font::Small)));

	SetRelativePositions();

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                                    TCredits                                              *
 ********************************************************************************************/

class TCredits : public TForm
{
/* Constructeur/Destructeur */
public:

	TCredits(ECImage*);

/* Composants */
public:

	TLabel*   Label1;
	TLabel*   Label2;
	TLabel*   Label3;
	TLabel*   Label4;
	TLabel*   Label5;
	TLabel*   Label7;
	TLabel*   Label6;
	TImage*   Title;
	TMemo*    Memo;

	TButtonText* OkButton;

	static void WantGoBack(TObject*, void*);

/* Variables */
public:
	bool want_goback;
};

void TCredits::WantGoBack(TObject* o, void*)
{
	TCredits* c = static_cast<TCredits*>(o->Parent());

	c->want_goback = true;
}

void MenAreAntsApp::WantCredits(TObject*, void*)
{
	TCredits*     Credits = new TCredits(Video::GetInstance()->Window());

	Credits->OkButton->SetOnClick(TCredits::WantGoBack, 0);

	Resources::DingDong()->Play();

	FORM_RUN(Credits, !Credits->want_goback);

	delete Credits;
	TMainForm::enter_in_main = true;
}

TCredits::TCredits(ECImage* w)
	: TForm(w), want_goback(false)
{
	Title = AddComponent(new TImage(300, 100, (Height()<800 ? Resources::TitleMini() : Resources::Title()), false));
	Title->SetXY(Width()/2 - Title->Width()/2, 50);

	Label1 = AddComponent(new TLabel(Title->Y()+Title->Height()+10,"Romain Bignon", red_color, Font::GetInstance(Font::Big)));
	Label2 = AddComponent(new TLabel(Label1->Y()+Label1->Height(),_("* Programmer"), red_color, Font::GetInstance(Font::Big)));

	Label3 = AddComponent(new TLabel(50,Label2->Y()+Label2->Height()+30,"Thomas Tourrette", fgreen_color, Font::GetInstance(Font::Big)));
	Label4 = AddComponent(new TLabel(50,Label3->Y()+Label3->Height(),_("* \"Graphiste\""), fgreen_color, Font::GetInstance(Font::Big)));

	Label5 = AddComponent(new TLabel(SCREEN_WIDTH-300,Label2->Y()+Label2->Height()+30,"Mathieu Nicolas", fwhite_color, Font::GetInstance(Font::Big)));
	Label6 = AddComponent(new TLabel(SCREEN_WIDTH-300,Label3->Y()+Label3->Height(),_("* Idea"), fwhite_color, Font::GetInstance(Font::Big)));

	OkButton = AddComponent(new TButtonText(SCREEN_WIDTH/2-75,SCREEN_HEIGHT-70, 150,50, _("Back"),
	                                        Font::GetInstance(Font::Normal)));

	Memo = AddComponent(new TMemo(Font::GetInstance(Font::Normal), 50, Label6->Y()+Label6->Height()+30, SCREEN_WIDTH-50-50,
	                              SCREEN_HEIGHT-(Label6->Y()+Label6->Height()+30)-OkButton->Height()-20, 0, false));
	Memo->SetShadowed();
	Memo->AddItem(_("Contributors:\n"
	                "\n"
	                "=Programmation=\n"
	                "- lodesi: patches for defense tower and other effects.\n"
	                "- phh: patche for the plane.\n"
	                "\n"
	                "=Others=\n"
	                "- Anicée: for her voice.\n"
	                "- Zic, Spouize, Nico, Mathieu and Thomas who have tested the game."), white_color);

	Memo->ScrollUp();

	SetBackground(Resources::Titlescreen());
}
