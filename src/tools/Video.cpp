/* src/tools/Video.h - Header of Video.cpp
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
 *
 * $Id$
 */
/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A ARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU GeneralPublic License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *****************************************************************************/

#include "Video.h"
#include <string>
#include <SDL_endian.h>
#include <SDL_image.h>
#include "Config.h"
#include "Main.h"
#include "Debug.h"

Video* Video::singleton = NULL;

Video* Video::GetInstance()
{
	if (singleton == NULL)
		singleton = new Video();

	return singleton;
}

Video::Video()
	: icon(0)
{
	SetMaxFps (50);
	fullscreen = false;
	SDLReady = false;
}

Video::~Video()
{
	if(icon)
		SDL_FreeSurface(icon);
	if( SDLReady )
		SDL_Quit();
}

void Video::SetMaxFps(uint max_fps)
{
	m_max_fps = max_fps;
	if (0 < m_max_fps)
		m_sleep_max_fps = 1000/m_max_fps;
	else
		m_sleep_max_fps = 0;
}

uint Video::MaxFps()
{
	return m_max_fps;
}

uint Video::SleepMaxFps()
{
	return m_sleep_max_fps;
}

bool Video::IsFullScreen(void) const
{
	return fullscreen;
}

bool Video::SetConfig(int width, int height, bool _fullscreen)
{
#ifndef BUGUED_FULLSCREEN /** In win32, when you are in fullscreen mode there is a matter with the cursor */
	_fullscreen = false;
#endif

	// initialize the main window
	if( window.IsNull() ||
			(width != window.GetWidth() ||
			 height != window.GetHeight() )
#ifdef WIN32
		|| fullscreen != _fullscreen
#endif
		 )
	{

		int flags = SDL_HWSURFACE | SDL_HWACCEL | SDL_DOUBLEBUF;

		if(_fullscreen) flags |= SDL_FULLSCREEN;

		window.SetImage( SDL_SetVideoMode(width, height, 32, flags), false );

		if(window.IsNull())
			window.SetImage( SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE) );

		if(window.IsNull())
			return false;
		fullscreen = _fullscreen;
	}
	else if(fullscreen != _fullscreen ){
		SDL_WM_ToggleFullScreen( window.Surface() );
		fullscreen = _fullscreen;
	}

	return true;
}

void Video::InitWindow()
{
	InitSDL();

	window.SetImage( NULL , false );
	window.SetAutoFree( false );

	Config *config = Config::GetInstance();
	SetConfig(config->screen_width,
			config->screen_height,
			config->fullscreen);

	if( window.IsNull() )
		throw ECExcept("", "Unable to initialize SDL window.");

	SetWindowCaption(MenAreAntsApp::GetInstance()->get_title());
}

void Video::SetWindowCaption(std::string caption)
{
	SDL_WM_SetCaption( caption.c_str(), NULL );
}

void Video::SetWindowIcon(std::string filename)
{
	if(icon) SDL_FreeSurface(icon);

	icon = IMG_Load(filename.c_str());
	SDL_WM_SetIcon(icon, NULL );
}

void Video::InitSDL()
{
	if( SDLReady )
		return;

	if( SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0 )
		throw ECExcept("", StringF("Unable to initialize SDL library: %s", SDL_GetError() ));

	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	SDLReady = true;
}

void Video::Flip()
{
	window.Flip();
}

