/* src/main.cpp - Main file
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

#include <SDL.h>
#include <SDL_image.h>
#include <cerrno>
#include <signal.h>

#ifndef WIN32
#include <sys/stat.h>
#include <dirent.h>
#else
#include <winsock2.h>
#endif

#include "Main.h"
#include "Defines.h"
#include "tools/Font.h"
#include "Config.h"
#include "tools/Video.h"
#include "tools/Images.h"
#include "Resources.h"
#include "Debug.h"
#include "gui/Form.h"
#include "gui/Boutton.h"
#include "gui/BouttonText.h"
#include "gui/Memo.h"
#include "gui/ComboBox.h"
#include "gui/Label.h"
#include "gui/Fps.h"
#include <functional>

extern const char* SVNVERSION;
#ifdef APP_VERSION_PATCH
std::string APP_VERSION = std::string(APP_VERSION_ALPHA "." APP_VERSION_BETA "(P" APP_PVERSION ")") + SVNVERSION +
                          std::string("-") + APP_VERSION_PATCH;
#else
std::string APP_VERSION = std::string(APP_VERSION_ALPHA "." APP_VERSION_BETA;
#endif

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
	MenAreAntsApp::GetInstance()->request_game();
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

void MenAreAntsApp::quit_app(int value)
{
#ifdef WIN32
		WSACleanup();
#endif
		Resources::Unload();
		TTF_Quit();
        exit(value);
}

int MenAreAntsApp::main(int argc, char **argv)
{
	try
	{
#ifndef WIN32
		signal(SIGPIPE, SIG_IGN);
#endif

		srand( (long)time(NULL) );

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

#ifdef WIN32
		WSADATA WSAData;
		if(WSAStartup(MAKEWORD(2,0), &WSAData) != 0)
			throw ECExcept(VIName(WSAGetLastError()), "Impossible d'initialiser les sockets windows");
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

		do
		{
			MainForm->Actions();
			if(TMainForm::enter_in_main)
			{
#ifdef BUGUED_INTRO
				TMainForm::enter_in_main = false;
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
			}
			MainForm->Update();
		} while(!want_quit);

		MyFree(MainForm);

		quit_app(1);
	}
	catch (const TECExcept &e)
	{
		std::cout << "Received an ECExcept error: " << std::endl;
		std::cout << e.Message() << std::endl;
#ifdef DEBUG
		std::cout << e.Vars() << std::endl;
#endif
		quit_app(225);
	}
#ifndef DEBUG
	catch (const std::exception &err)
	{
		std::cout << std::endl << "Exception caught from STL:" << std::endl;
		std::cout << err.what() << std::endl;
		quit_app(255);
	}
#endif

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
	PlayButton = AddComponent(new TButton(300,150, 150,50));
	PlayButton->SetImage(new ECSprite(Resources::PlayButton(), Video::GetInstance()->Window()));

	OptionsButton = AddComponent(new TButton(300,230, 150,50));
	OptionsButton->SetImage(new ECSprite(Resources::OptionsButton(), Video::GetInstance()->Window()));

	MapEditorButton = AddComponent(new TButtonText(300,310, 150,50, "Editeur de maps", Font::GetInstance(Font::Normal)));
	/** \todo utiliser une image bouton comme pour les autres
	 MapEditorButton = AddComponent(new TButton(300,310, 150,50));
	 MapEditorButton->SetImage(new ECSprite(Resources::MapEditorButton(), app.sdlwindow));
	 */

	CreditsButton = AddComponent(new TButton(300,390, 150,50));
	CreditsButton->SetImage(new ECSprite(Resources::CreditsButton(), Video::GetInstance()->Window()));

	QuitterButton = AddComponent(new TButton(300,470, 150,50));
	QuitterButton->SetImage(new ECSprite(Resources::QuitterButton(), Video::GetInstance()->Window()));

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

	do
	{
		Credits->Actions();
		Credits->Update();
	} while(!Credits->want_goback);

	delete Credits;
	TMainForm::enter_in_main = true;
}

TCredits::TCredits(ECImage* w)
	: TForm(w), want_goback(false)
{
	Label1 = AddComponent(new TLabel(105,"Romain Bignon", red_color, Font::GetInstance(Font::Big)));
	Label2 = AddComponent(new TLabel(135,"* Programmeur", red_color, Font::GetInstance(Font::Big)));

	Label3 = AddComponent(new TLabel(50,205,"Thomas Tourrette", blue_color, Font::GetInstance(Font::Big)));
	Label4 = AddComponent(new TLabel(50,235,"* Graphiste", blue_color, Font::GetInstance(Font::Big)));

	Label5 = AddComponent(new TLabel(SCREEN_WIDTH-300,205,"Mathieu Nicolas", fwhite_color, Font::GetInstance(Font::Big)));
	Label6 = AddComponent(new TLabel(SCREEN_WIDTH-300,235,"* Idée originale", fwhite_color, Font::GetInstance(Font::Big)));
	Label7 = AddComponent(new TLabel(SCREEN_WIDTH-300,265,"* Musique", fwhite_color, Font::GetInstance(Font::Big)));

	Memo = AddComponent(new TMemo(Font::GetInstance(Font::Normal), 50, 340, SCREEN_WIDTH-50-50, 190, 0, false));
	Memo->AddItem("Merci au lycée Corneilles pour nous avoir mis dans le contexte emmerdant qui "
	              "nous a permit de trouver des idées \"amusantes\" pour passer le temps et qui "
	              "aboutirent à ce jeu en version plateau que l'on pu experimenter pendant les "
	              "cours d'histoire et d'espagnol. Je tiens d'ailleurs à remercier le manque "
	              "d'autorité de Mme Gay (histoire) et Mme Goetz (espagnol).\n"
	              "\n"
                  "Merci également à Zic, Spouize, Nico, Mathieu, Thomas et Anicée pour avoir testé le jeu.", white_color);
	Memo->ScrollUp();

	OkButton = AddComponent(new TButtonText(SCREEN_WIDTH/2-75,SCREEN_HEIGHT-70, 150,50, "Retour", Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
