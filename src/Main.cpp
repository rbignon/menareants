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

#include <ClanLib/Core/System/system.h>
#include <ClanLib/Display/setupdisplay.h>
#include <ClanLib/Core/System/setupcore.h>
#include <ClanLib/Core/System/error.h>
#include <ClanLib/Core/Resources/datafile_compiler.h>
#include <ClanLib/Display/Input/input.h>
#include <ClanLib/Display/Input/inputbuffer.h>
#include <ClanLib/Display/Input/keyboard.h>
#include <ClanLib/Display/Input/key.h>
#include <ClanLib/Display/Display/display.h>
#include <ClanLib/Display/Display/surface.h>
#include <ClanLib/Core/IOData/inputsource.h>
#include <ClanLib/Display/Font/font.h>
#include <ClanLib/display.h>

#ifndef WIN32
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "Main.h"
#include "Menu.h"

void EuroConqApp::quit_app(int value)
{
		deinit_modules();
		if(conf) delete conf;
        exit(value);
}

int EuroConqApp::main(int argc, char **argv)
{
	try
	{
		init_modules();
		bool fullscreen = false;
#ifdef WIN32
		fullscreen = true;
#endif

		if (argc > 1) {
			if (strcmp (argv[1], "-datafile") == 0) {
				CL_DatafileCompiler::write("euroconq.scr", "euroconq.dat");
				quit_app(0);
			}
			else if (strcmp (argv[1], "-fullscreen") == 0) {
				fullscreen = true;
			}
			else if (strcmp (argv[1], "-window") == 0) {
				fullscreen = false;
			}
			else if (strcmp (argv[1], "-h") == 0 ||
					strcmp (argv[1], "-help") == 0 ||
					strcmp (argv[1], "--help") == 0 ||
					strcmp (argv[1], "-?") == 0 ||
					strcmp (argv[1], "/?") == 0 ||
					strcmp (argv[1], "/h") == 0) {
				cout << "usage: " << argv[0] << " [-fullscreen | -window]" << endl;
				quit_app(0);
			}
		}

		srand( (long)time(NULL) );
		Resources::init();

		CL_Display::set_videomode(800, 600, 16, fullscreen);

		CL_Display::clear_display();
		CL_Display::flip_display();
		CL_Display::clear_display();
		CL_Display::flip_display();
		CL_MouseCursor::hide();

#ifndef WIN32
		if (getenv("HOME"))
		{
			CL_String path;
			path = getenv("HOME");
			path += "/.euroconq";
			if (!opendir(path))
			{
				mkdir( path, 0755 );
			}

			conf = new Config( path + "/euroconq.cfg" );
		}
#else
		conf = new Config("euroconq.cfg");
#endif

		CL_Display::clear_display();
		Resources::Font_big()->print_center( 400, 300, "Loading..." );
		CL_Display::flip_display();
		Resources::load_all();
		conf->load();

#if 0
		CL_Surface *Image = CL_PNGProvider::create("pics/prout.png", NULL);

		while(!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
		{
			CL_Display::clear_display();
			Image->put_screen(0, 0);
			CL_Display::flip_display();
			CL_System::keep_alive();
		}

		delete Image;
#endif

		menu = new Menu( CL_String("Menu principal"), this);
		menu->add_item(CL_String("Jouer"), MENU_JOUER, &connect_to_server, 0);
			menu->add_item(  CL_String("Creer une partie"), JOUER_CREER, M_READ_ONLY, MENU_JOUER );
			menu->add_item(  CL_String("Lister les parties"), JOUER_LISTER, M_READ_ONLY, MENU_JOUER );
			menu->add_item(  CL_String("Se deconnecter"), JOUER_RETOUR, M_RETOUR, MENU_JOUER);
		menu->add_item(CL_String("Options"), MENU_OPTIONS, 0);
			menu->add_string(CL_String("Serveur"), OPTIONS_HOST, M_READ_ONLY|M_NOFMAJ, MENU_OPTIONS, conf->hostname);
			menu->add_value( CL_String("Port"), OPTIONS_PORT, M_READ_ONLY, MENU_OPTIONS, 100, 65535, conf->port);
			menu->add_string(CL_String("Pseudo"), OPTIONS_NICK, 0, MENU_OPTIONS, conf->nick );
			menu->add_item(  CL_String("Retour"), OPTIONS_RETOUR, M_RETOUR, MENU_OPTIONS);
		menu->add_item(CL_String("Quitter"), MENU_EXIT, 0 );

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
			catch (CL_Error err)
			{
				Menu menu_err( "Shit", this);
				menu_err.add_item(err.message, 0, 0);
				menu_err.scroll_in();
				menu_err.execute();
				menu_err.scroll_out();
			}
		}

		quit_app(1);
	}
	catch (CL_Error err)
	{
		std::cout << std::endl << "Exception caught from ClanLib:" << std::endl;
		std::cout << err.message << std::endl;
		quit_app(255);
	}

	return 0;

}

EuroConqApp app;
