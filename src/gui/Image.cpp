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

TImage::TImage()
	: TComponent()
{
	image = 0;
	wana_delete = true;
}

TImage::TImage(int _x, int _y, ECImage* _img, bool _wana_delete)
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

	size.x = image->Img->w;
	size.y = image->Img->h;

	SetWantRedraw();
}

void TImage::SetImage(ECImage* _img, bool _wd)
{
	Hide();
	if(wana_delete && image)
		delete image;

	image = _img;
	wana_delete = _wd;
	Init();
	Show();
}

void TImage::Draw(const Point2i& mouse)
{
	if(!image || image->IsNull() || !Window()) return;

	Window()->Blit(image, position);
}
