/* src/gui/Image.cpp - TImage  component
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
 *
 * $Id$
 */

#include "gui/Image.h"
#include "Debug.h"
#include "Main.h"

TImage::TImage()
	: TComponent()
{
	image = 0;
	wana_delete = true;
}

TImage::TImage(uint _x, uint _y, ECImage* _img, bool _wana_delete)
	: TComponent(_x, _y), image(_img), wana_delete(_wana_delete)
{

}

TImage::~TImage()
{
	if(wana_delete && image) delete image;
}

void TImage::Init()
{
	if(!image)
		return;

	w = image->Img->w;
	h = image->Img->h;
}

void TImage::SetImage(ECImage* _img, bool _wd)
{
	image = _img;
	wana_delete = _wd;
	Init();
}

void TImage::Draw(uint _x, uint _y)
{
	if(!image) return;
	SDL_Rect r_back = {x,y,w,h};
	SDL_BlitSurface( image->Img, NULL, app.sdlwindow, &r_back);
}
