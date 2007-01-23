/* src/gui/Image.h - Header of Image.cpp
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

#ifndef EC_GUI_IMAGE_H
#define EC_GUI_IMAGE_H

#include <SDL.h>
#include "tools/Images.h"
#include "gui/Component.h"

/** This component show an image */
class TImage : public TComponent
{
/* Constructeur/Destructeur */
public:

	TImage();
	TImage(int x, int y, ECImage* _img = 0, bool wana_delete = true);
	~TImage();

/* Methodes */
public:

	virtual void Init();

	virtual void Draw (const Point2i&);

/* Attributs */
public:

	ECImage* Image() const { return image; }
	void SetImage(ECImage* _img, bool wana_delete = true);/**< Set maximal value */

	virtual bool RedrawBackground() const { return false; }

/* Variables privées */
protected:
	ECImage* image;

	/* Interdits*/
	void SetHeight (uint _h) {}                           /**< Set \a height */
	void SetWidth (uint _w) {}                            /**< Set \a width */
	bool wana_delete;
};

#endif /* EC_GUI_IMAGE_H */
