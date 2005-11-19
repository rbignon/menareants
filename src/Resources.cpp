/* src/Resources.cpp - Struct which define all resources.
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

#include "Resources.h"
#include "Defines.h"

#ifdef WIN32
#	define PKGDATADIR
#endif

ECImage*				Resources::sur_titlescreen = NULL;
ECImage*				Resources::sur_loadscreen = NULL;


Resources::Resources()
{
}

Resources::~Resources()
{

}

void Resources::Unload()
{
	if(sur_titlescreen) delete sur_titlescreen;
	if(sur_loadscreen) delete sur_loadscreen;
}

ECImage* Resources::Titlescreen()
{
	if (!sur_titlescreen)
		sur_titlescreen = new ECImage(PKGDATADIR_PICS "menu.png");
	return sur_titlescreen;

}

ECImage* Resources::Loadscreen()
{
	if (!sur_loadscreen)
		sur_loadscreen = new ECImage(PKGDATADIR_PICS "loading.png");
	return sur_loadscreen;

}
