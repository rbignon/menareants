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

#include <stdlib.h>

#include <ClanLib/Display/Font/font.h>
#include <ClanLib/Core/Resources/resource_manager.h>
#include <ClanLib/Core/System/error.h>
#include <ClanLib/Display/Display/surface.h>
#include <ClanLib/Sound/soundbuffer.h>

#include "Resources.h"

#ifdef WIN32
#	define PKGDATADIR
#endif

CL_ResourceManager*	Resources::res = NULL;

CL_Font*			Resources::fnt_big = NULL;
CL_Font*			Resources::fnt_small = NULL;

CL_Surface*			Resources::sur_titlescreen = NULL;
CL_Surface*			Resources::sur_loadscreen = NULL;


Resources::Resources()
{
}

Resources::~Resources()
{
}

void Resources::init()
{
	try
	{
		res = new CL_ResourceManager(PKGDATADIR "euroconq.dat", true);
	}
	catch (CL_Error err)
	{
		try
		{
			res = new CL_ResourceManager(
				"/usr/share/euroconq/euroconq.dat",
				true);
		}
		catch (CL_Error err)
		{
			res = new CL_ResourceManager(
				"/usr/local/share/euroconq/euroconq.dat",
				true);
		}
	}
}

void Resources::load_all()
{
	res->load_all();
}

CL_Font* Resources::Font_big()
{
	if (!fnt_big)
	{
		fnt_big = CL_Font::load("Font/big", res);
	}
	return fnt_big;
}

CL_Font* Resources::Font_small()
{
	if (!fnt_small)
	{
		fnt_small = CL_Font::load("Font/small", res);
	}
	return fnt_small;
}

CL_Surface* Resources::Titlescreen()
{
	if (!sur_titlescreen)
		sur_titlescreen = CL_Surface::load("Titlescreen/main", res);
	return sur_titlescreen;

}

CL_Surface* Resources::Loadscreen()
{
	if (!sur_loadscreen)
		sur_loadscreen = CL_Surface::load("Titlescreen/loading", res);
	return sur_loadscreen;

}
