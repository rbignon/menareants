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

#include "tools/Images.h"

class Resources
{
public:
	Resources();
	~Resources();

	static ECImage*	Titlescreen();
	static ECImage*	Loadscreen();
	static ECImage* Menuscreen();

	static ECSpriteBase* UpButton();
	static ECSpriteBase* DownButton();
	static ECSpriteBase* NormalButton();

	static void Unload();

protected:

	static ECImage *sur_titlescreen;
	static ECImage *sur_loadscreen;
	static ECImage *sur_menuscreen;

	static ECSpriteBase *spr_upbutton;
	static ECSpriteBase *spr_downbutton;
	static ECSpriteBase *spr_normalbutton;
};

#endif
