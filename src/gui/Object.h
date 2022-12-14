/* src/gui/Object.h - Declaration of TObject
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

#ifndef EC_OBJECT_H
#define EC_OBJECT_H

class ECImage;

/********************************************************************************************
 *                                  TObject                                                 *
 ********************************************************************************************/

class TObject
{
public:
	TObject()
		: parent(0), window(0)
	{}

	TObject(ECImage* w)
		: parent(0), window(w)
	{}

	virtual ~TObject() {}

	void SetParent(TObject* o) { parent = o; }
	TObject* Parent() const { return parent; }

	void SetWindow(ECImage* w) { window = w; }
	ECImage* Window() const { return window; }

private:
	TObject* parent;
	ECImage* window;
};

#endif /* EC_OBJECT_H */
