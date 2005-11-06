/* src/Resources.h - Header of Resources.cpp
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

#ifndef RESOURCES_H
#define RESOURCES_H

class CL_Font;
class CL_Surface;
#if 0 /* TODO: later */
class CL_SoundBuffer;
class CL_SoundBuffer_Session;
#endif
class CL_ResourceManager;

/**
  *@author Andreas Hundt, Denis Oliver Kropp
  */

class Resources
{
public:
	Resources();
	~Resources();

	static void init();
	static void load_all();

	static CL_Font*		Font_big();
	static CL_Font*		Font_small();

	static CL_Surface*              Titlescreen();

	static CL_ResourceManager *res;

protected:
	static CL_Font *fnt_big;
	static CL_Font *fnt_small;

	static CL_Surface *sur_titlescreen;
};

#endif
