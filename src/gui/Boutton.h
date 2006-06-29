/* src/gui/Boutton.h - Header of Boutton.cpp
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

#ifndef EC_BOUTTON_H
#define EC_BOUTTON_H

#include "Component.h"
#include "tools/Images.h"

/** This is a component based on TComponent and whose show a button with a picture
 *
 * \note There isn't any text, you have to see \a TButtonText
 */
class TButton : public TComponent
{
/* Constructeur/Destructeur */
public:
	TButton();
	TButton (int x, int y, unsigned int w, unsigned int h);
	~TButton();

/* Methodes */
public:
	void Init();

	virtual void Draw (int souris_x, int souris_y);

/* Attributs */
public:

	unsigned int Tag;

	virtual void SetImage (ECSprite *image);

/* Variables protégées */
protected:

	ECSprite *image;

	void DrawImage (int souris_x, int souris_y);
};


#endif /* EC_BOUTTON_H */
