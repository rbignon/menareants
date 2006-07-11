/* src/tools/Images.h - Header of Images.cpp
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
 * $Id$
 */

#include <SDL_rotozoom.h>
#include "Images.h"
#include "Video.h"
#include "Debug.h"
#include "tools/Color.h"

void DrawRect(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color)
{
    int x, y;

    for(y = y1; y <= y2; y++)
    {
        putpixel(screen, x1, y, color);
        putpixel(screen, x2, y, color);
    }

    for(x = x1; x <= x2; x++)
    {
        putpixel(screen, x, y1, color);
        putpixel(screen, x, y2, color);
    }
}

void DrawLine(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color)
{
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    // Are these safety checks really needed?
    if(x1 > screen->w)
        x1 = screen->w - 1;

    if(x2 > screen->w)
        x2 = screen->w - 1;

    if(y1 > screen->h)
        y1 = screen->h - 1;

    if(y2 > screen->h)
        y2 = screen->h - 1;

    dx = x2 - x1;               /* the horizontal distance of the line */
    dy = y2 - y1;               /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    if(dxabs >= dyabs)          /* the line is more horizontal than vertical */
    {
        for(i = 0; i < dxabs; i++)
        {
            y += dyabs;
            if(y >= dxabs)
            {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            if(px < 0) continue;
            putpixel(screen, px, py, color);
        }
    }
    else                        /* the line is more vertical than horizontal */
    {
        for(i = 0; i < dyabs; i++)
        {
            x += dxabs;
            if(x >= dyabs)
            {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            if(py < 0) continue;
            putpixel(screen, px, py, color);
        }
    }
}

SDL_Surface* CreateRGBSurface (int width, int height, Uint32 flags){

  SDL_Surface* surface = SDL_CreateRGBSurface(flags, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
          0xff000000,  // red mask
          0x00ff0000,  // green mask
          0x0000ff00,  // blue mask
#else
          0x000000ff,  // red mask
          0x0000ff00,  // green mask
          0x00ff0000,  // blue mask
#endif
          0 // don't use alpha
   );
  if ( surface == NULL )
      throw ECExcept(0, std::string("Can't create SDL RGBA surface: ") + SDL_GetError());
  return surface;
}

SDL_Surface* CreateRGBASurface (int width, int height, Uint32 flags){

  SDL_Surface* surface = SDL_CreateRGBSurface(flags, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
          0xff000000,  // red mask
          0x00ff0000,  // green mask
          0x0000ff00,  // blue mask
          0x000000ff // alpha mask
#else
          0x000000ff,  // red mask
          0x0000ff00,  // green mask
          0x00ff0000,  // blue mask
          0xff000000 // alpha mask
#endif
   );
  if ( surface == NULL )
      throw ECExcept(0, std::string("Can't create SDL RGBA surface: ") + SDL_GetError());
  return surface;
}

void ChangePixelColor(ECImage* surf, Color last_color, Color new_color)
{
	SLOCK(surf->Img);
	for(uint x = 0; x < surf->GetWidth(); x++)
		for(uint y = 0; y < surf->GetHeight(); y++)
		{
			Uint32 col = getpixel(surf->Img, x, y);
			/*Uint8 r, g, b;
			SDL_GetRGB(col, surf->Img->format, &r, &g, &b);*/
			if(last_color == surf->GetColor(col))
				putpixel(surf->Img, x, y, surf->MapColor(new_color));
		}
	SUNLOCK(surf->Img);
}

Uint32 getpixel(SDL_Surface * surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *) p;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *) p;

        default:
            return 0;
    }
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    if(x<0 || y<0 || x >= surface->w || y >= surface->h) return;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


/****************************************************************************************
 *                                      ECSprite                                        *
 ****************************************************************************************/

int ECSprite::init(ECSpriteBase *base, ECImage *screen)
{
  assert(base);
  assert(screen);
  mAnimating = false;
  mFrame = 0;
  mDrawn = 0;
  mSpeed = 1;
  mLastupdate = 0;
  order = true;
  repeat = true;
  mX = 0; mY = 0; mOldX = 0; mOldY = 0;

  //mSpriteBase = new ECSpriteBase(base->path.c_str());
  mSpriteBase = base;

  if(mSpriteBase->mNumframes>1 && mSpriteBase->animation) mAnimating=true;

  mScreen = screen;
  return 0;
}

ECSprite::~ECSprite()
{

}

bool ECSprite::Anim() const { return (mSpriteBase->animation && mAnimating); }

void ECSprite::SetFrame(uint nr)
{
	mFrame = nr < mSpriteBase->mNumframes ? nr : mSpriteBase->mNumframes-1;
}

uint ECSprite::NbFrames() const
{
	return mSpriteBase ? mSpriteBase->mNumframes : 0;
}

ECImage* ECSprite::First() const
{
	if(mSpriteBase && mSpriteBase->mNumframes)
		return &mSpriteBase->mAnim[0];
	return 0;
}

void ECSprite::draw()
{
	if(mAnimating && mSpriteBase->animation)
	{
		if(mLastupdate + mSpriteBase->mAnim[mFrame].pause * mSpeed<SDL_GetTicks())
		{
			if(order)
			{
				if(mFrame >= mSpriteBase->mNumframes-1)
				{
					if(!repeat)
						mAnimating = false;
					else
						mFrame=0;
				}
				else
					mFrame++;
			}
			else
			{
				if(!mFrame)
				{
					if(!repeat)
						mAnimating = false;
					else
						mFrame = mSpriteBase->mNumframes-1;
				}
				else mFrame--;
			}
			mLastupdate = SDL_GetTicks();
		}
	}
	
	if(mDrawn==0) mDrawn=1;
	
	SDL_Rect dest;
	dest.x = mX;
	dest.y = mY;
	dest.w = GetWidth();
	dest.h = GetHeight();
	mScreen->Blit(&mSpriteBase->mAnim[mFrame], &dest);
}

uint ECSprite::GetWidth()
{
	return (mSpriteBase ? mSpriteBase->mW : 0);
}
uint ECSprite::GetHeight()
{
	return (mSpriteBase ? mSpriteBase->mH : 0);
}

void ECSprite::ChangeColor(Color first, Color to)
{
	mSpriteBase->ChangeColor(first, to);
}

void ECSprite::RotoZoom(double angle, double zoomx, double zoomy, bool smooth)
{
	mSpriteBase->RotoZoom(angle, zoomx, zoomy, smooth);
}

void ECSprite::Zoom(double zoomx, double zoomy, bool smooth)
{
	mSpriteBase->Zoom(zoomx, zoomy, smooth);
}


/****************************************************************************************
 *                                      ECSpriteBase                                    *
 ****************************************************************************************/

ECSpriteBase::ECSpriteBase(const char *dir)
{
	mAnim = 0;
	mBuilt = 0;
	mNumframes = 0;
	mW = 0;
	mH = 0;
	animation = false;

	init(dir);
}

ECSpriteBase::~ECSpriteBase()
{
	if(mAnim)
		delete [] mAnim;
}

int ECSpriteBase::init(const char *dir)
{
  char buffer[255];
  char filename[255];
  char name[255];
  int pause=0, r=0, g=0, b=0;
  FILE *fp;

  sprintf(filename, PKGDATADIR_ANIMS "%s/info", dir);

  if((fp=fopen(filename, "r")) == NULL)
    throw ECExcept(filename, "Problème d'ouverture des données. Vérifiez leur présence");

  fgets(buffer, 255, fp);
  sscanf(buffer, "FILES: %d", &mNumframes);
  mAnim = new ECImage[mNumframes];

  mBuilt = 1;

  for(uint count=0;!feof(fp) && count<mNumframes;)
  {
    fgets(buffer, 255, fp);
    if(buffer[0] != '#' && buffer[0] != '\r' && buffer[0] != '\0' && buffer[0] != '\n')
    {
      sscanf(buffer, "%s %d %d %d %d", name, &pause, &r, &g, &b);
      sprintf(filename, PKGDATADIR_ANIMS "%s/%s", dir, name);
      SDL_Surface *temp;
      if((temp = IMG_Load(filename)) == NULL)
         throw ECExcept(filename, "Impossible de charger une image.");
      if(r >= 0) SDL_SetColorKey(temp, SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, r, g, b));
      mAnim[count].Img = SDL_DisplayFormat(temp);
      SDL_FreeSurface(temp);

      mAnim[count].pause = pause;
      if(pause) animation = true;
      if(!mW) mW = mAnim[count].Img->w;
      if(!mH) mH = mAnim[count].Img->h;

      count++;
    }
  }
  fclose(fp);
  path = dir;
  return 0;
}

void ECSpriteBase::ChangeColor(Color from, Color to)
{
	for(uint i=0; i < mNumframes; ++i)
		ChangePixelColor(&mAnim[i], from, to);
}

void ECSpriteBase::RotoZoom(double angle, double zoomx, double zoomy, bool smooth)
{
	for(uint i=0; i < mNumframes; ++i)
	{
		mAnim[i].RotoZoom(angle, zoomx, zoomy, smooth);
		mW = mAnim[i].Img->w;
		mH = mAnim[i].Img->h;
 }
}

void ECSpriteBase::Zoom(double zoomx, double zoomy, bool smooth)
{
	for(uint i=0; i < mNumframes; ++i)
	{
		mAnim[i].Zoom(zoomx, zoomy, smooth);
		mW = mAnim[i].Img->w;
		mH = mAnim[i].Img->h;
	}
}

ECImage* ECSpriteBase::First() const
{
	if(mNumframes)
		return &mAnim[0];
	return 0;
}

/****************************************************************************************
 *                                      ECImage                                         *
 ****************************************************************************************/

ECImage::ECImage(char* fichier)
	: pause(0), autofree(true)
{
	Load(fichier);
}

void ECImage::Free()
{
	if(Img && autofree)
	{
		SDL_FreeSurface(Img);
		Img = 0;
	}
}


ECImage::~ECImage()
{
	Free();
}

ECImage &ECImage::operator=(const ECImage & src){
        Free();
        Img = src.Img;
        if( Img != NULL )
                Img->refcount++;

        return *this;
}

int ECImage::Blit(const ECImage& src, SDL_Rect *srcRect, SDL_Rect *dstRect)
{
	return SDL_BlitSurface( src.Img, srcRect, Img, dstRect );
}

int ECImage::Blit(const ECImage& src, SDL_Rect *dstRect)
{
	return SDL_BlitSurface( src.Img, 0, Img, dstRect );
}

int ECImage::Blit(const ECImage& src)
{
	return SDL_BlitSurface( src.Img, 0, Img, 0);
}

int ECImage::Blit(const ECImage* src, SDL_Rect *srcRect, SDL_Rect *dstRect)
{
	return SDL_BlitSurface( src->Img, srcRect, Img, dstRect );
}

int ECImage::Blit(const ECImage* src, SDL_Rect *dstRect)
{
	return SDL_BlitSurface( src->Img, 0, Img, dstRect );
}

int ECImage::Blit(const ECImage* src)
{
	return SDL_BlitSurface( src->Img, 0, Img, 0);
}

void ECImage::Load(char *fichier)
{
	SDL_Surface *tmp = 0;
	if(!(tmp = IMG_Load(fichier)))
		throw ECExcept(VSName(fichier), "Impossible d'ouvrir le fichier image");
	Img = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	SetColorKey(255,0,255);
}

void ECImage::Draw(int x, int y)
{
  if(!Img) return;
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  Video::GetInstance()->Window()->Blit(this, &dest);
}

void ECImage::Draw(int x, int y, int w, int h, int x2, int y2)
{
  if(!Img) return;
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_Rect dest2;
  dest2.x = x2;
  dest2.y = y2;
  dest2.w = w;
  dest2.h = h;
  Video::GetInstance()->Window()->Blit(this, &dest2, &dest);
}

void ECImage::Draw()
{
  Draw(0, 0);
}

void ECImage::SetColorKey(unsigned int r, unsigned int g, unsigned int b)
{
	if(!Img) return;
	SDL_SetColorKey(Img, SDL_SRCCOLORKEY, SDL_MapRGB(Img->format, r, g, b));
}

void ECImage::GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a)
{
	if(!Img) return;
	SDL_GetRGBA(color, Img->format, &r, &g, &b, &a);
}

Uint32 ECImage::MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(!Img) return 0;
    return SDL_MapRGBA(Img->format, r, g, b, a);
}

Color ECImage::GetColor(Uint32 color)
{
	Uint8 r, g, b, a;
	GetRGBA(color, r, g, b, a);
	return Color(r, g, b, a);
}

Uint32 ECImage::MapColor(Color color)
{
	if(!Img) return 0;
	return MapRGBA(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

void ECImage::Flip()
{
	if(!Img) return;
	SDL_Flip( Img );
}

int ECImage::Fill(Uint32 color)
{
	if(!Img) return 0;
	return SDL_FillRect( Img, NULL, color);
}

int ECImage::Fill(const Color &color)
{
	if(!Img) return 0;
	return Fill( MapColor(color) );
}

int ECImage::FillRect(SDL_Rect &dstRect, Uint32 color)
{
	if(!Img) return 0;
	return SDL_FillRect( Img, &dstRect, color);
}

int ECImage::FillRect(SDL_Rect &dstRect, const Color color)
{
	if(!Img) return 0;
	return SDL_FillRect( Img, &dstRect, MapColor(color));
}

void ECImage::NewSurface(uint width, uint height, Uint32 flags, bool useAlpha)
{
	Uint32 alphaMask;
	Uint32 redMask;
	Uint32 greenMask;
	Uint32 blueMask;

	if( autofree )
		Free();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	redMask = 0xff000000;
	greenMask = 0x00ff0000;
	blueMask = 0x0000ff00;
	alphaMask =	0x000000ff;
#else
	redMask = 0x000000ff;
	greenMask = 0x0000ff00;
	blueMask = 0x00ff0000;
	alphaMask = 0xff000000;
#endif

	if( !useAlpha )
		alphaMask = 0;

	Img = SDL_CreateRGBSurface(flags, width, height, 32,
			redMask, greenMask, blueMask, alphaMask );

	if( Img == NULL )
		throw ECExcept("", std::string("Can't create SDL RGBA surface: ") + SDL_GetError() );
}

void ECImage::RotoZoom(double angle, double zoomx, double zoomy, bool smooth)
{
#ifndef WIN32
	SetImage( rotozoomSurfaceXY(Img, angle, zoomx, zoomy, smooth) );

	if( IsNull() )
		throw ECExcept("", "Unable to make a rotozoom on the surface !" );
#endif
}

void ECImage::Zoom(double zoomx, double zoomy, bool smooth)
{
#ifndef WIN32
	SetImage( zoomSurface(Img, zoomx, zoomy, smooth) );

	if(IsNull() )
		throw ECExcept("", "Unable to make a rotozoom on the surface !" );
#endif
}
