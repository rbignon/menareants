/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 *  Graphic effects on sprite
 *****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "effects_wave.h"

const float tmin = 100.0;
const float tmax = 300.0;

const float WaveEffect::t(const unsigned int frame)
{
  return tmin + ((float)frame/(float)nbr_frames) * (tmax-tmin);
}

const float WaveEffect::Wave(const float d, const unsigned int frame)
{
  // Wave shape
  return cos(cos(d) * nbr_wave * t(frame) * M_PI / tmax);
}

const float WaveEffect::Gaussian(const float d, const unsigned int frame)
{
  // Gaussian shape (flat at t=0, increase linearly until t=tmax/2,
  // then dicrease to flat until t=tmax)
  return ( 1/sqrt(2*M_PI) ) * exp(-d*d) * 2 *
         ( (t(frame)<tmax/2) ? (t(frame)*2/tmax) : ((tmax-t(frame))*2/tmax) );
}

const float WaveEffect::CenterDst(int x, int y)
{
  // Returns the distance from (x,y) to the center of the image
  // (aka pythagore..)
  if(size_y/2 < radius)
    y += (int)radius - size_y/2;
   if(size_x/2 < radius)
    x += (int)radius - size_x/2;
  float d  =  sqrt( (radius - x)*(radius - x)
                  + (radius - y)*(radius - y));
  d /= radius;
  assert( d >= 0.0 && d <= sqrt(2));
  return d;
}

const float WaveEffect::Height(const float d, const unsigned int frame)
{
  float h = ((Wave(d, frame) * Gaussian(d, frame))/2.0) + 0.5;
  assert( h>=0.0 && h<=1.0 );
  return h;
}

const float WaveEffect::Length(const float d, const unsigned int frame)
{
  float delta = 1.0 / radius;
  float d0 = 0.0;
  float h0 = Height(d0, frame);
  float lenght = 0.0;
  for(float d1 = delta; d1<=d ; d1 += delta)
  {
    float h1 = Height(d1,frame);
    float dh = sqrt((h1 - h0)*(h1 - h0)
                  +(d1 - d0)*(d1 - d0));
    lenght += (dh > 0.0) ? dh : -dh;
    h0 = h1;
    d0 = d1;
  }
  return lenght;
}

ECSpriteBase* WaveEffect::Wave3dSurface(ECImage &a, unsigned int _nbr_frames, unsigned int _duration, float _nbr_wave)
{
  nbr_frames = _nbr_frames;
  duration = _duration;
  nbr_wave = _nbr_wave;
  size_x = a.GetWidth();
  size_y = a.GetHeight();
  radius = (float) (size_x > size_y)?size_x:size_y;
  radius /= 2.0;

  int center_x = size_x / 2;
  int center_y = size_y / 2;

  a.Lock();
  ECSpriteBase* anim = new ECSpriteBase();
  anim->mW = size_x;
  anim->mH = size_y;
  for(unsigned int frame = 0; frame < nbr_frames ; frame++)
  {
    float len_total = Length(1.0, frame);

    ECImage f(Point2i(size_x, size_y), SDL_SWSURFACE);
    f.Lock();
    unsigned char* buf = f.GetPixels();
    for(int y=0; y < size_y; y++)
    for(int x=0; x < size_x; x++)
    {
      float d = CenterDst(x, y);
      float len = Length(d, frame);

      float d_pix = (len/len_total) * radius;

      Point2i tmp = Point2i(x - center_x, y - center_y);
      float angle = tmp.ComputeAngle();

      int src_x = center_x + int(cos(angle) * d_pix);
      int src_y = center_y + int(sin(angle) * d_pix);

      Uint32 col;
      unsigned char* col_buf = (unsigned char*) &col;
      if(src_x < 0
      || src_y < 0
      || src_x >= size_x
      || src_y >= size_y)
      {
        col_buf[0] = SDL_ALPHA_TRANSPARENT;
        col_buf[1] = 0;
        col_buf[2] = 0;
        col_buf[3] = 0;
      }
      else
      {
        col = a.GetPixel(src_x, src_y);
	float h = Height(d, frame);
	if(h < 0.5)
	{
	  col_buf[1] *= (h + 0.5);
	  col_buf[2] *= (h + 0.5);
	  col_buf[3] *= (h + 0.5);
	}
      }
      memcpy(buf, &col, 4);
      buf += 4;
    }
    f.Unlock();
    f.SetPause (duration / nbr_frames);
    anim->surfaces.push_back(f);
    //anim->AddFrame(f);
  }
  //anim->mSpeed = (duration / nbr_frames);
  a.Unlock();
  return anim;
}

