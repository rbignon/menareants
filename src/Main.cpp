/* src/main.cpp - Main file
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

#include <SDL.h>
#include <SDL_image.h>
#include <cerrno>

#ifndef WIN32
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "Main.h"
#include "Menu.h"
#include "tools/Font.h"
#include "tools/Images.h"

EuroConqApp app;

void EuroConqApp::quit_app(int value)
{
		if(conf) delete conf;
		Resources::Unload();
        exit(value);
}

int EuroConqApp::main(int argc, char **argv)
{
	try
	{
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
			return 0;
		}
		atexit(SDL_Quit);
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

  		app.sdlwindow = SDL_SetVideoMode(800,600,32, sdlflags);
  		SDL_WM_SetCaption(get_title(), NULL);

#ifndef WIN32
		if (getenv("HOME"))
		{
			std::string path;
			path = getenv("HOME");
			path += "/.euroconq";
			if (!opendir(path.c_str()))
			{
				mkdir( path.c_str(), 0755 );
			}

			conf = new Config( path + "/euroconq.cfg" );
		}
#else
		conf = new Config("euroconq.cfg");
#endif

		ECImage *loading_image = Resources::Loadscreen();

		loading_image->Draw();

		if (TTF_Init()==-1) {
			std::cerr << "TTF_Init: "<< TTF_GetError() << std::endl;
			return false;
		}

		Font::InitAllFonts();

		SDL_UpdateRect(app.sdlwindow, 0, 0, 0, 0);
		SDL_Flip(app.sdlwindow);

		conf->load();

		menu = new Menu( std::string("Menu principal"), this);
		menu->add_item(std::string("Jouer"), MENU_JOUER, &connect_to_server, 0);
			menu->add_item(  std::string("Creer une partie"), JOUER_CREER, M_READ_ONLY, MENU_JOUER );
			menu->add_item(  std::string("Lister les parties"), JOUER_LISTER, M_READ_ONLY, MENU_JOUER );
			menu->add_item(  std::string("Se deconnecter"), JOUER_RETOUR, M_RETOUR, MENU_JOUER);
		menu->add_item(std::string("Options"), MENU_OPTIONS, 0);
			menu->add_string(std::string("Serveur"), OPTIONS_HOST, M_READ_ONLY|M_NOFMAJ, MENU_OPTIONS, conf->hostname);
			menu->add_value( std::string("Port"), OPTIONS_PORT, M_READ_ONLY, MENU_OPTIONS, 100, 65535, conf->port);
			menu->add_string(std::string("Pseudo"), OPTIONS_NICK, 0, MENU_OPTIONS, conf->nick );
			menu->add_item(  std::string("Retour"), OPTIONS_RETOUR, M_RETOUR, MENU_OPTIONS);
		menu->add_item(std::string("Quitter"), MENU_EXIT, 0 );

		menu->scroll_in();

		while (1)
		{
			int result = menu->execute();

			try
			{
				MenuItem* item = menu->get_item_by_id( result );
				switch (result)
				{
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
			catch (const std::exception &err)
			{
				Menu menu_err( "Shit", this);
				menu_err.add_item(err.what(), 0, 0);
				menu_err.scroll_in();
				menu_err.execute();
				menu_err.scroll_out();
			}
		}

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
