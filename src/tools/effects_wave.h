/* src/tools/effects_wave.h - Wave effect
 *
 * Copyright (C) 2007 Laurent Defert
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

#ifndef EFFECTS_WAVE_H
#define EFFECTS_WAVE_H

#include <SDL.h>
#include "tools/Images.h"

class WaveEffect
{
	unsigned int nbr_frames;
	unsigned int duration;
	float nbr_wave;
	float radius;
	int size_x;
	int size_y;

	float t(const unsigned int frame);
	float Wave(const float d, const unsigned int frame);
	float Gaussian(const float d, const unsigned int frame);
	float CenterDst(int x, int y);
	float Height(const float d, const unsigned int frame);
	float Length(const float d, const unsigned int frame);
public:
	ECSpriteBase* Wave3dSurface(ECImage &a, unsigned int _nbr_frames, unsigned int _duration, float _nbr_wave);
};


#endif //EFFECTS_H
