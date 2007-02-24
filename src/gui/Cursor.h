/* src/gui/Cursor.h - Header of Cursor.cpp
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

#ifndef EC_CURSOR_H
#define EC_CURSOR_H

#include <map>
#include "gui/Component.h"

class TMap;

class TCursor : public TComponent
{
/* Constructeur/Destructeur */
public:

	TCursor();
	~TCursor();

	enum cursors_t {
		TOPLEFT_POINTERS,
		Standard,
		MIDDLE_POINTERS,
		Select,
		Attaq,
		MaintainedAttaq,
		CantAttaq,
		Invest,
		Left,
		Radar,
		AddBP,
		RemBP,
	};
	typedef std::map<cursors_t, ECImage*> ImgList;

/* Methodes */
public:

	void Init();

	void Draw(const Point2i&);

/* Attributs */
public:

	cursors_t Pointer() const { return pointer; }

	void SetCursor(cursors_t);

	void SetCursorImage(cursors_t i, ECImage* b);

	TMap* Map() const { return map; }
	void SetMap(TMap* m) { map = m; }

/* Variables privées */
private:
	bool initialized;
	TMap* map;
	cursors_t pointer;
	ImgList cursors;
};

extern TCursor Cursor;

#endif /* EC_CURSOR_H */
