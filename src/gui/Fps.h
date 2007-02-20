/* src/gui/Fps.h - Header of Fps.cpp
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

#ifndef EC_FPS_H
#define EC_FPS_H

#include <list>
#include "Component.h"
#include "Label.h"

class Font;

class TFPS : public TLabel
{
private:
  // Minimum number of values needed to compute the average
  static const uint MIN_NB_VALUES;

  int nb_valid_values;
  double average;
  std::list<uint> nb_frames;
  uint time_in_second;
  TLabel text;

public:
  bool display;

public:
	TFPS(int x, int y, Font*);

	void Init();

	void Reset();
	void AddOneFrame();
	void Draw(const Point2i&);
};

#endif /* EC_FPS_H */
