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
#include <getopt.h>
#include <winsock2.h>
#endif

#include "Main.h"
#include "tools/Font.h"
#include "tools/Images.h"
#include "Resources.h"
#include "Debug.h"
#include "gui/Form.h"
#include "gui/Boutton.h"
#include "gui/BouttonText.h"
#include "gui/Memo.h"
#include "gui/ComboBox.h"
#include "gui/Label.h"
#include <functional>

class TMainForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TMainForm(SDL_Surface*);
	~TMainForm();

/* Composants */
public:

	TButton*    PlayButton;
	TButton*    OptionsButton;
	TButton*    CreditsButton;
	TButton*    QuitterButton;
	TButtonText* MapEditorButton;
	TLabel*     Version;

/* Evenements */
public:

};

MenAreAntsApp app;

char* MenAreAntsApp::get_title()
{
	return APP_NAME;
}

void MenAreAntsApp::WantQuit(TObject*, void*)
{
	app.want_quit = true;
}

void MenAreAntsApp::WantPlay(TObject*, void*)
{
	app.request_game();
}

void MenAreAntsApp::WantMapEditor(TObject*, void*)
{
	app.MapEditor();
}

void MenAreAntsApp::WantConfig(TObject*, void* b)
{
	if(app.conf)
		app.conf->Configuration((bool)b);
}

void MenAreAntsApp::setclient(EC_Client* c)
{
	client = c;
	c->lapp = this;
}

void MenAreAntsApp::quit_app(int value)
{
#ifdef WIN32
		WSACleanup();
#endif
		if(conf) delete conf;
		if(fonts) delete fonts;
		Resources::Unload();
		TTF_Quit();
		SDL_Quit();
        exit(value);
}

int MenAreAntsApp::main(int argc, char **argv)
{
	try
	{
#ifndef WIN32
		signal(SIGPIPE, SIG_IGN);
#endif
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
			return 0;
		}
		SDL_EnableUNICODE(1);
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
		int sdlflags = SDL_SWSURFACE|SDL_HWPALETTE;
#ifdef WIN32
		sdlflags |= SDL_FULLSCREEN;
#endif

		for(int tmp = 0; (tmp = getopt(argc, argv, "hwf")) != EOF;)
			switch(tmp)
			{
				case 'f':
					sdlflags |= SDL_FULLSCREEN;
					break;
				case 'w':
					sdlflags &= ~SDL_FULLSCREEN;
					break;
				case 'h':
				default:
					std::cout << "Usage: " << argv[0] << " [-wf]" << std::endl;
					quit_app(EXIT_FAILURE);
			}

		srand( (long)time(NULL) );

  		app.sdlwindow = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,32, sdlflags);
  		SDL_WM_SetCaption(get_title(), NULL);

		ECImage *loading_image = Resources::Loadscreen();

		loading_image->Draw();

#ifndef WIN32
		path = GetHome();
		path += "/.menareants/";
		DIR *d;
		if(!(d = opendir(path.c_str())))
			mkdir( path.c_str(), 0755 );
		else closedir(d);

		conf = new Config( path + CONFIG_FILE );
#else
		conf = new Config(CONFIG_FILE);
#endif

		if (TTF_Init()==-1) {
			std::cerr << "TTF_Init: "<< TTF_GetError() << std::endl;
			return false;
		}

		fonts = new Fonts;

#ifdef WIN32
		WSADATA WSAData;
		if(WSAStartup(MAKEWORD(2,0), &WSAData) != 0)
			throw ECExcept(VIName(WSAGetLastError()), "Impossible d'initialiser les sockets windows");
#endif

		SDL_UpdateRect(app.sdlwindow, 0, 0, 0, 0);
		SDL_Flip(app.sdlwindow);

		conf->load();

		if(first_run)
			MenAreAntsApp::WantConfig(0,(void*)true);

		TMainForm* MainForm = new TMainForm(sdlwindow);
		MainForm->PlayButton->SetOnClick(MenAreAntsApp::WantPlay, this);
		MainForm->QuitterButton->SetOnClick(MenAreAntsApp::WantQuit, this);
		MainForm->CreditsButton->SetOnClick(MenAreAntsApp::WantCredits, this);
		MainForm->MapEditorButton->SetOnClick(MenAreAntsApp::WantMapEditor, this);
		MainForm->OptionsButton->SetOnClick(MenAreAntsApp::WantConfig, (void*)false);

		do
		{
			MainForm->Actions();
			MainForm->Update();
		} while(!want_quit);

		delete MainForm;

		quit_app(1);
	}
	catch (const TECExcept &e)
	{
		std::cout << "Received an ECExcept error: " << std::endl;
		std::cout << e.Message << std::endl;
#ifdef DEBUG
		std::cout << e.Vars << std::endl;
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
  return app.main(argc,argv);
}

/********************************************************************************************
 *                                    TMainForm                                             *
 ********************************************************************************************/

TMainForm::TMainForm(SDL_Surface* w)
	: TForm(w)
{
	PlayButton = AddComponent(new TButton(300,150, 150,50));
	PlayButton->SetImage(new ECSprite(Resources::PlayButton(), app.sdlwindow));

	OptionsButton = AddComponent(new TButton(300,230, 150,50));
	OptionsButton->SetImage(new ECSprite(Resources::OptionsButton(), app.sdlwindow));

	MapEditorButton = AddComponent(new TButtonText(300,310, 150,50, "Editeur de maps", &app.Font()->normal));
	/** \todo utiliser une image bouton comme pour les autres
	 MapEditorButton = AddComponent(new TButton(300,310, 150,50));
	 MapEditorButton->SetImage(new ECSprite(Resources::MapEditorButton(), app.sdlwindow));
	 */

	CreditsButton = AddComponent(new TButton(300,390, 150,50));
	CreditsButton->SetImage(new ECSprite(Resources::CreditsButton(), app.sdlwindow));

	QuitterButton = AddComponent(new TButton(300,470, 150,50));
	QuitterButton->SetImage(new ECSprite(Resources::QuitterButton(), app.sdlwindow));

	Version = AddComponent(new TLabel(750,105,APP_VERSION, white_color, &app.Font()->big));
	Version->SetXY(Version->X() - Version->Width(), Version->Y());

	SetBackground(Resources::Titlescreen());
}

TMainForm::~TMainForm()
{
	delete Version;
	delete QuitterButton;
	delete CreditsButton;
	delete MapEditorButton;
	delete OptionsButton;
	delete PlayButton;
}

/********************************************************************************************
 *                                    TCredits                                              *
 ********************************************************************************************/

class TCredits : public TForm
{
/* Constructeur/Destructeur */
public:

	TCredits(SDL_Surface*);
	~TCredits();

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
	TCredits*     Credits = new TCredits(app.sdlwindow);

	Credits->OkButton->SetOnClick(TCredits::WantGoBack, 0);

	do
	{
		Credits->Actions();
		Credits->Update();
	} while(!Credits->want_goback);

	delete Credits;
}

TCredits::TCredits(SDL_Surface* w)
	: TForm(w), want_goback(false)
{
	Label1 = AddComponent(new TLabel(300,105,"Romain Bignon", red_color, &app.Font()->big));
	Label2 = AddComponent(new TLabel(300,135,"* Programmeur", red_color, &app.Font()->big));

	Label3 = AddComponent(new TLabel(50,205,"Thomas Tourrette", blue_color, &app.Font()->big));
	Label4 = AddComponent(new TLabel(50,235,"* Graphiste", blue_color, &app.Font()->big));

	Label5 = AddComponent(new TLabel(500,205,"Mathieu Nicolas", fwhite_color, &app.Font()->big));
	Label6 = AddComponent(new TLabel(500,235,"* Idée originale", fwhite_color, &app.Font()->big));
	Label7 = AddComponent(new TLabel(500,265,"* Musique", fwhite_color, &app.Font()->big));

	Memo = AddComponent(new TMemo(&app.Font()->normal, 50, 340, SCREEN_WIDTH-50-50, 190, 0, false));
	Memo->AddItem("Merci au lycée Corneilles pour nous avoir mis dans le contexte emmerdant qui "
	              "nous a permit de trouver des idées \"amusantes\" pour passer le temps et qui "
	              "aboutirent à ce jeu en version plateau que l'on pu experimenter pendant les "
	              "cours d'histoire et d'espagnol. Je tiens d'ailleurs à remercier le manque "
	              "d'autorité de Mme Gay (histoire) et Mme Goetz (espagnol).\n"
	              "\n"
                  "Merci également à Zic, Spouize, Nico, Mathieu, Thomas et Anicée pour avoir testé le jeu.", white_color);
	Memo->ScrollUp();

	OkButton = AddComponent(new TButtonText(SCREEN_WIDTH/2-75,530, 150,50, "Retour", &app.Font()->normal));

	SetBackground(Resources::Titlescreen());
}

TCredits::~TCredits()
{
	delete OkButton;
	delete Memo;
	delete Label7;
	delete Label6;
	delete Label5;
	delete Label4;
	delete Label3;
	delete Label2;
	delete Label1;
}
