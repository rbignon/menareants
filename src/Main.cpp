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
#endif

#include "Main.h"
#include "tools/Font.h"
#include "tools/Images.h"
#include "Resources.h"
#include "Debug.h"
#include "gui/Form.h"
#include "gui/Boutton.h"
#include <functional>

class TMainForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TMainForm();
	~TMainForm();

/* Composants */
public:

	TButton*    PlayButton;
	TButton*    OptionsButton;
	TButton*    CreditsButton;
	TButton*    QuitterButton;

/* Evenements */
public:

};

EuroConqApp app;

void EuroConqApp::WantQuit(void*, void*)
{
	app.want_quit = true;
}

void EuroConqApp::WantPlay(void*, void*)
{
	app.request_game();
}

void EuroConqApp::WantConfig(void*, void*)
{
	if(app.conf)
		app.conf->Configuration();
}

void EuroConqApp::setclient(EC_Client* c)
{
	client = c;
	c->lapp = this;
}

void EuroConqApp::quit_app(int value)
{
		if(conf) delete conf;
		if(fonts) delete fonts;
		Resources::Unload();
        exit(value);
}

int EuroConqApp::main(int argc, char **argv)
{
	try
	{
		signal(SIGPIPE, SIG_IGN);
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
			return 0;
		}
		atexit(SDL_Quit);
		SDL_EnableUNICODE(1);
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
		int sdlflags = SDL_SWSURFACE|SDL_HWPALETTE;
#ifdef WIN32
		sdlflags |= SDL_FULLSCREEN;
#endif

		if (argc > 1) {
			if (strcmp (argv[1], "-fullscreen") == 0) {
				sdlflags |= SDL_FULLSCREEN;
			}
			else if (strcmp (argv[1], "-window") == 0) {
				sdlflags &= ~SDL_FULLSCREEN;
			}
			else if (strcmp (argv[1], "-h") == 0 ||
					strcmp (argv[1], "-help") == 0 ||
					strcmp (argv[1], "--help") == 0 ||
					strcmp (argv[1], "-?") == 0 ||
					strcmp (argv[1], "/?") == 0 ||
					strcmp (argv[1], "/h") == 0) {
				std::cout << "usage: " << argv[0] << " [-fullscreen | -window]" << std::endl;
				quit_app(0);
			}
		}

		srand( (long)time(NULL) );

  		app.sdlwindow = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,32, sdlflags);
  		SDL_WM_SetCaption(get_title(), NULL);

		ECImage *loading_image = Resources::Loadscreen();

		loading_image->Draw();

#ifndef WIN32
		if (getenv("HOME"))
		{
			path = getenv("HOME");
			path += "/.euroconq/";
			if (!opendir(path.c_str()))
			{
				mkdir( path.c_str(), 0755 );
			}

			conf = new Config( path + "euroconq.cfg" );
		}
		else
		{
			std::cout << "Unable to read HOME environment variable !" << std::endl;
			quit_app(0);
		}
#else
		conf = new Config("euroconq.cfg");
#endif

		if (TTF_Init()==-1) {
			std::cerr << "TTF_Init: "<< TTF_GetError() << std::endl;
			return false;
		}

		fonts = new Fonts;

		SDL_UpdateRect(app.sdlwindow, 0, 0, 0, 0);
		SDL_Flip(app.sdlwindow);

		conf->load();

		TMainForm*     MainForm = new TMainForm;
		MainForm->PlayButton->SetClickedFunc(EuroConqApp::WantPlay, this);
		MainForm->QuitterButton->SetClickedFunc(EuroConqApp::WantQuit, this);
		MainForm->OptionsButton->SetClickedFunc(EuroConqApp::WantConfig, this);

		do
		{
			MainForm->Actions();
			MainForm->Update();
		} while(!want_quit);

#if 0
		while (1)
		{
			int result = menu->execute();

			try
			{
				MenuItem* item = menu->get_item_by_id( result );
				switch (result)
				{
					case MENU_JOUER:
						request_game(); /* Entre dans le menu de jeu (socket) */
						break;
					case MENU_EXIT:
						menu->scroll_out();
						delete menu;
						quit_app(0);
						break;
	     			case OPTIONS_HOST:
    	 				conf->hostname = ((MenuItem_String*)item)->get_string();
     					conf->save();
     					break;
     				case OPTIONS_PORT:
    	 				conf->port = ( ((MenuItem_Value*)item)->get_value() );
     					conf->save();
     					break;
     				case OPTIONS_NICK:
    	 				conf->nick = ((MenuItem_String*)item)->get_string();
     					conf->save();
     					break;
     				case -1: break; /**normal**/
					default:
						std::cout << result << std::endl;
	     				break;
				}
			}
			catch(TECExcept &e)
			{
				vDebug(W_ERR, e.Message, e.Vars);
				Menu menu_err( "Shit", this);
				menu_err.add_item(e.Message, 0, 0);
				menu_err.scroll_in();
				menu_err.execute();
				menu_err.scroll_out();
			}
		}
#endif
		quit_app(1);
	}

	catch (const std::exception &err)
	{
		std::cout << std::endl << "Exception caught from STL:" << std::endl;
		std::cout << err.what() << std::endl;
		quit_app(255);
	}

	return 0;

}

int main (int argc, char **argv)
{
  app.main(argc,argv);
}

/********************************************************************************************
 *                                    TMainForm                                             *
 ********************************************************************************************/

TMainForm::TMainForm()
	: TForm()
{
	PlayButton = AddComponent(new TButton(300,150, 150,50));
	PlayButton->SetImage(new ECSprite(Resources::PlayButton(), app.sdlwindow));

	OptionsButton = AddComponent(new TButton(300,250, 150,50));
	OptionsButton->SetImage(new ECSprite(Resources::OptionsButton(), app.sdlwindow));

	CreditsButton = AddComponent(new TButton(300,350, 150,50));
	CreditsButton->SetImage(new ECSprite(Resources::CreditsButton(), app.sdlwindow));

	QuitterButton = AddComponent(new TButton(300,450, 150,50));
	QuitterButton->SetImage(new ECSprite(Resources::QuitterButton(), app.sdlwindow));

	SetBackground(Resources::Titlescreen());
}

TMainForm::~TMainForm()
{
	delete QuitterButton;
	delete CreditsButton;
	delete OptionsButton;
	delete PlayButton;
}
