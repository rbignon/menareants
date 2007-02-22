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
#include "Sound.h"
#include "Version.h"
#include "gui/Boutton.h"
#include "gui/BouttonText.h"
#include "gui/ComboBox.h"
#include "gui/Form.h"
#include "gui/Fps.h"
#include "gui/Label.h"
#include "gui/Memo.h"
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

	TButtonText* PlayButton;
	TButtonText* OptionsButton;
	TButtonText* CreditsButton;
	TButtonText* QuitterButton;
	TButtonText* MapEditorButton;
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
	PlayButton->SetXY(Window()->GetWidth()/2 - PlayButton->Width()/2, 150 * Window()->GetHeight() / 600);
	OptionsButton->SetXY(Window()->GetWidth()/2 - OptionsButton->Width()/2, 230 * Window()->GetHeight() / 600);
	MapEditorButton->SetXY(Window()->GetWidth()/2 - MapEditorButton->Width()/2, 310 * Window()->GetHeight() / 600);
	CreditsButton->SetXY(Window()->GetWidth()/2 - CreditsButton->Width()/2, 390 * Window()->GetHeight() / 600);
	QuitterButton->SetXY(Window()->GetWidth()/2 - QuitterButton->Width()/2, 470 * Window()->GetHeight() / 600);
	Version->SetXY(Window()->GetWidth()-50 - Version->Width(), Version->Y());
}

TMainForm::TMainForm(ECImage* w)
	: TForm(w)
{
	PlayButton = AddComponent(new TButtonText(300,150, 150,50, _("Play"), Font::GetInstance(Font::Normal)));

	OptionsButton = AddComponent(new TButtonText(300,230, 150,50, _("Options"), Font::GetInstance(Font::Normal)));

	MapEditorButton = AddComponent(new TButtonText(300,310, 150,50, _("Map Editor"), Font::GetInstance(Font::Normal)));

	CreditsButton = AddComponent(new TButtonText(300,390, 150,50, _("Credits"), Font::GetInstance(Font::Normal)));

	QuitterButton = AddComponent(new TButtonText(300,470, 150,50, _("Exit"), Font::GetInstance(Font::Normal)));

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
	Label1 = AddComponent(new TLabel(105,"Romain Bignon", red_color, Font::GetInstance(Font::Big)));
	Label2 = AddComponent(new TLabel(135,_("* Programmer"), red_color, Font::GetInstance(Font::Big)));

	Label3 = AddComponent(new TLabel(50,205,"Thomas Tourrette", fgreen_color, Font::GetInstance(Font::Big)));
	Label4 = AddComponent(new TLabel(50,235,_("* \"Graphiste\""), fgreen_color, Font::GetInstance(Font::Big)));

	Label5 = AddComponent(new TLabel(SCREEN_WIDTH-300,205,"Mathieu Nicolas", fwhite_color, Font::GetInstance(Font::Big)));
	Label6 = AddComponent(new TLabel(SCREEN_WIDTH-300,235,_("* Idea"), fwhite_color, Font::GetInstance(Font::Big)));

	Memo = AddComponent(new TMemo(Font::GetInstance(Font::Normal), 50, 340, SCREEN_WIDTH-50-50, 190, 0, false));
	Memo->SetShadowed();
	Memo->AddItem("Merci au lycée Corneille pour nous avoir mis dans le contexte emmerdant qui "
	              "nous a permis de trouver des idées \"amusantes\" pour passer le temps et qui "
	              "aboutirent à ce jeu en version plateau que l'on pu experimenter pendant les "
	              "cours d'histoire et d'espagnol.\n"
	              "\n"
	              "Merci à lodesi pour ses patchs.\n"
	              "Merci à Anicée pour sa voix.\n"
	              "Merci à Cesar pour ne pas avoir participé à la programmation du jeu.\n"
	              "\n"
                  "Merci également à Zic, Spouize, Nico, Mathieu, et Thomas pour avoir testé le jeu.", white_color);
	Memo->ScrollUp();

	OkButton = AddComponent(new TButtonText(SCREEN_WIDTH/2-75,SCREEN_HEIGHT-70, 150,50, _("Back"),
	                                        Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
