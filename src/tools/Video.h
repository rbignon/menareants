/* src/tools/Video.cpp - Video tools
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *****************************************************************************/

#ifndef EC_VIDEO_H
#define EC_VIDEO_H

#include <SDL.h>
#include <string>
#include "tools/Images.h"

class Video
{
/* Constructeur/Destructeur */
public:
	Video();
	~Video();

/* Methodes */
public:

	void InitWindow(void);
	void InitSDL(void);
	void Flip(void);

	bool SetConfig(uint width, uint height, bool fullscreen);

/* Attributs */
public:

	void SetWindowIcon(std::string icon);
	void SetWindowCaption(std::string caption);
	void SetMaxFps(uint max_fps);
	uint MaxFps();
	uint SleepMaxFps();
	ECImage* Window() { return &window; }
	uint Height() { return window.GetHeight(); }
	uint Width() { return window.GetWidth(); }
	bool IsFullScreen(void) const;

/* Variables privées */
private:
	uint m_max_fps;     // If equals to zero, it means no limit
	uint m_sleep_max_fps;
	bool SDLReady;
	bool fullscreen;

	ECImage window;
	
public:

	static Video* singleton;
	static Video* GetInstance();
};

#endif /* EC_VIDEO_H */
