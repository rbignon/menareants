/* src/main.h - Main file
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

#include "Resources.h"

#include <ClanLib/Core/System/setupcore.h>
#include <ClanLib/Display/setupdisplay.h>
#include <ClanLib/Sound/setupsound.h>
#include <ClanLib/Application/clanapp.h>
#include <ClanLib/Core/System/clanstring.h>
#include <ClanLib/png.h>
#include "link.h"

#include "Defines.h"

#ifndef WIN32
	#include <config.h>
#endif


class Menu;

enum {
	MENU_JOUER,
	MENU_EXIT,

	JOUER_LISTER,
	JOUER_CREER
};

class EuroConqApp : public CL_ClanApplication
{
protected:
	Menu* menu;
public:
	virtual int main(int argc, char** argv);
	virtual char *get_title() { return APP_NAME; }
#ifndef WIN32
	virtual void quit_app(int value) __attribute__ ((noreturn));
#else
	virtual void quit_app(int value);
#endif

	virtual void init_modules()
	{
		CL_SetupCore::init();
		CL_SetupPNG::init();
		CL_SetupDisplay::init();
	}

	virtual void deinit_modules()
	{
		CL_SetupCore::deinit();
		CL_SetupDisplay::deinit();
		CL_SetupPNG::deinit();
	}

	static bool connect_to_server()
	{
		std::cout << "prout" << std::endl;
		return false;
	}
};
