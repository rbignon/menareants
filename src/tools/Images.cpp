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

#include "tools/SDL_rotozoom.h"
#include "Images.h"
#include "Video.h"
#include "Debug.h"
#include "tools/Color.h"
#include <fstream>
#include <SDL_gfxPrimitives.h>

void DrawLargeLine(SDL_Surface* screen, int x1, int y1, int x2, int y2, Uint32 color)
{
	int _x1, _y1, _x2, _y2;

	for(_x1 = x1-1, _x2 = x2-1; _x1 != x1+1; _x1++, _x2++)
		for(_y1 = y1-1, _y2 = y2-1; _y1 != y1+1; _y1++, _y2++)
			DrawLine(screen, _x1, _y1, _x2, _y2, color);
}

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
	for(int x = 0; x < surf->GetWidth(); x++)
		for(int y = 0; y < surf->GetHeight(); y++)
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

  if(mSpriteBase->NumFrames()>1 && mSpriteBase->animation) mAnimating=true;

  mScreen = screen;
  return 0;
}

ECSprite::~ECSprite()
{

}

bool ECSprite::Anim() const { return (mSpriteBase->animation && mAnimating); }

void ECSprite::SetFrame(uint nr)
{
	mFrame = nr < mSpriteBase->NumFrames() ? nr : mSpriteBase->NumFrames()-1;
}

uint ECSprite::NbFrames() const
{
	return mSpriteBase ? mSpriteBase->NumFrames() : 0;
}

ECImage* ECSprite::First() const
{
	if(mSpriteBase && mSpriteBase->surfaces.empty() == false)
		return &mSpriteBase->surfaces.front();
	return 0;
}

void ECSprite::draw()
{
	if(mAnimating && mSpriteBase->animation)
	{
		if(mLastupdate + mSpriteBase->surfaces[mFrame].pause * mSpeed<SDL_GetTicks())
		{
			if(order)
			{
				if(mFrame >= mSpriteBase->NumFrames()-1)
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
						mFrame = mSpriteBase->NumFrames()-1;
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
	mScreen->Blit(mSpriteBase->surfaces[mFrame], &dest);
}

int ECSprite::GetWidth()
{
	return (mSpriteBase ? mSpriteBase->mW : 0);
}
int ECSprite::GetHeight()
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

ECSpriteBase::ECSpriteBase()
	: mBuilt(0), mW(0), mH(0), animation(false), alpha(false)
{

}

ECSpriteBase::ECSpriteBase(const char *dir)
	: mBuilt(0), mW(0), mH(0), animation(false), alpha(false)
{
	init(dir);
}

ECSpriteBase::~ECSpriteBase()
{

}

int ECSpriteBase::init(const char *dir)
{
	std::string filename;
	int pause=0, r=0, g=0, b=0;

	filename = PKGDATADIR_ANIMS + std::string(dir) + "/info";

	std::ifstream fp(filename.c_str());

	if(!fp)
		throw ECExcept(filename, "Problème d'ouverture des données. Vérifiez leur présence");

	std::string ligne, key;
	while(std::getline(fp, ligne))
	{
		if(ligne[0] == '#' || ligne[0] == '\r' || ligne[0] == '\0' || ligne[0] == '\n')
			continue;

		std::string key = stringtok(ligne, " ");

		if(key == "FILES:") continue; // On ignore
		else
		{
			filename = PKGDATADIR_ANIMS + std::string(dir) + PATH_SEPARATOR + key;
			bool a = false;
			pause = StrToTyp<int>(stringtok(ligne, " "));

			if(!ligne.empty())
			{
				r = StrToTyp<int>(stringtok(ligne, " "));
				g = StrToTyp<int>(stringtok(ligne, " "));
				b = StrToTyp<int>(stringtok(ligne, " "));
			}
			else
			{
				a = true;
				alpha = true;
			}

			SDL_Surface *temp;
			ECImage img;
			if((temp = IMG_Load(filename.c_str())) == NULL)
				throw ECExcept(filename, "Impossible de charger l'image: " + filename);

			if(!a)
				SDL_SetColorKey(temp, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(temp->format, r, g, b));

			img.SetImage(a ? SDL_DisplayFormatAlpha(temp) : SDL_DisplayFormat(temp));
			img.SetAlpha(a);
			SDL_FreeSurface(temp);

			img.pause = pause;
			if(pause) animation = true;
			if(!mW) mW = img.Img->w;
			if(!mH) mH = img.Img->h;

			surfaces.push_back(img);
		}
	}

	path = dir;
	return 0;
}

void ECSpriteBase::ChangeColor(Color from, Color to)
{
	for(uint i=0; i < NumFrames(); ++i)
		ChangePixelColor(&surfaces[i], from, to);
}

void ECSpriteBase::RotoZoom(double angle, double zoomx, double zoomy, bool smooth)
{
	for(uint i=0; i < NumFrames(); ++i)
	{
		surfaces[i].RotoZoom(angle, zoomx, zoomy, smooth);
		mW = surfaces[i].Img->w;
		mH = surfaces[i].Img->h;
	}
}

void ECSpriteBase::Zoom(double zoomx, double zoomy, bool smooth)
{
	for(uint i=0; i < NumFrames(); ++i)
	{
		surfaces[i].Zoom(zoomx, zoomy, smooth);
		mW = surfaces[i].Img->w;
		mH = surfaces[i].Img->h;
	}
}

ECImage* ECSpriteBase::First()
{
	if(surfaces.empty() == false)
		return &surfaces.front();
	return 0;
}

/****************************************************************************************
 *                                      ECImage                                         *
 ****************************************************************************************/

ECImage::ECImage(const char* fichier, bool _alpha)
	: Img(0), shadowed(0), pause(0), autofree(true), alpha(_alpha), x(0), y(0)
{
	Load(fichier, _alpha);
}

ECImage::ECImage(const Point2i &size, Uint32 flags, bool useAlpha)
	: Img(0), shadowed(0), pause(0), autofree(true), alpha(useAlpha), x(0), y(0)
{
	NewSurface(size, flags, useAlpha);
}

void ECImage::Load(const char *fichier, bool _alpha)
{
	SDL_Surface *tmp = 0;
	if(!(tmp = IMG_Load(fichier)))
		throw ECExcept(VSName(fichier), "Impossible d'ouvrir le fichier image");

	alpha = _alpha;
	Img = alpha ? SDL_DisplayFormatAlpha(tmp) : SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

	if(!alpha)
		SetColorKey(255,0,255);
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
	delete shadowed;
}

ECImage::ECImage(const ECImage & src)
	: shadowed(0), pause(src.pause), autofree(true), alpha(src.alpha), x(src.x), y(src.y)
{
	Img = src.Img;
	if(Img)
		Img->refcount++;
}

ECImage &ECImage::operator=(const ECImage & src)
{
	assert(this != &src);
	Free();
	Img = src.Img;
	autofree = true;
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

int ECImage::Blit(const ECImage& src, const Point2i &dst)
{
	SDL_Rect dstRect = GetSDLRect( dst, Point2i(src.GetWidth(), src.GetHeight()) );

	return Blit(src, NULL, &dstRect);
}

int ECImage::Blit(const ECImage& src, const Rectanglei &srcRect, const Point2i &dstPoint)
{
	SDL_Rect sdlSrcRect = GetSDLRect( srcRect );
	SDL_Rect sdlDstRect = GetSDLRect( dstPoint, Point2i(src.GetWidth(), src.GetHeight()) );

	return Blit(src, &sdlSrcRect, &sdlDstRect);
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

int ECImage::Blit(const ECImage* src, const Point2i &dst)
{
	SDL_Rect dstRect = GetSDLRect( dst, Point2i(src->GetWidth(), src->GetHeight()) );

	return Blit(src, NULL, &dstRect);
}

int ECImage::Blit(const ECImage* src, const Rectanglei &srcRect, const Point2i &dstPoint)
{
	SDL_Rect sdlSrcRect = GetSDLRect( srcRect );
	SDL_Rect sdlDstRect = GetSDLRect( dstPoint, Point2i(src->GetWidth(), src->GetHeight()) );

	return Blit(src, &sdlSrcRect, &sdlDstRect);
}

SDL_Rect ECImage::GetSDLRect(const Rectanglei &r) const
{
  SDL_Rect sdlRect;

  sdlRect.x = r.X();
  sdlRect.y = r.Y();
  sdlRect.w = r.Width();
  sdlRect.h = r.Height();

  return sdlRect;
}

SDL_Rect ECImage::GetSDLRect(const Point2i &pt, const Point2i &pt2) const
{
  SDL_Rect sdlRect;

  sdlRect.x = pt.X();
  sdlRect.y = pt.Y();
  sdlRect.w = pt2.X();
  sdlRect.h = pt2.Y();

  return sdlRect;
}

void ECImage::Draw(int x, int y)
{
  if(!Img) return;
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = GetWidth();
  dest.h = GetHeight();
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
  Draw(x, y);
}

void ECImage::SetColorKey(unsigned int r, unsigned int g, unsigned int b)
{
	if(!Img || alpha) return;
	SDL_SetColorKey(Img, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(Img->format, r, g, b));
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

void ECImage::NewSurface(const Point2i& size, Uint32 flags, bool useAlpha)
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

	Img = SDL_CreateRGBSurface(flags, size.x, size.y, 32,
			redMask, greenMask, blueMask, alphaMask );

	if( Img == NULL )
		throw ECExcept("", std::string("Can't create SDL RGBA surface: ") + SDL_GetError() );
}

void ECImage::RotoZoom(double angle, double zoomx, double zoomy, bool smooth)
{
	SetImage( rotozoomSurfaceXY(Img, angle, zoomx, zoomy, smooth) );

	if( IsNull() )
		throw ECExcept("", "Unable to make a rotozoom on the surface !" );
}

void ECImage::Zoom(double zoomx, double zoomy, bool smooth)
{
	SetImage( zoomSurface(Img, zoomx, zoomy, smooth) );

	if(IsNull() )
		throw ECExcept("", "Unable to make a rotozoom on the surface !" );
}

int ECImage::BoxColor(const Rectanglei &rect, const Color &color)
{
	if( rect.IsSizeZero() )
		return 0;

	Point2i ptBR = rect.GetBottomRightPoint();

	return boxRGBA( Img, rect.X(), rect.Y(), ptBR.X(), ptBR.Y(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int ECImage::RectangleColor(const Rectanglei &rect, const Color &color, const uint &border_size)
{
	if( rect.IsSizeZero() )
		return 0;

	Point2i ptBR = rect.GetBottomRightPoint();

	if (border_size == 1)
		return rectangleRGBA( Img, rect.X(), rect.Y(), ptBR.X(), ptBR.Y(), color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

	// top border
	boxRGBA (Img,
		rect.X(), rect.Y(), ptBR.X(), rect.Y()+border_size,
		color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

	// bottom border
	boxRGBA (Img,
		rect.X(), ptBR.Y() - border_size, ptBR.X(), ptBR.Y(),
		color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

	// left border
	boxRGBA (Img,
		rect.X(), rect.Y() + border_size, rect.X()+border_size, ptBR.Y()-border_size,
		color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

	// right border
	boxRGBA (Img,
		ptBR.X() - border_size, rect.Y() + border_size, ptBR.X(), ptBR.Y()-border_size,
		color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );

	return 1;
}

int ECImage::VlineColor(const uint &x1, const uint &y1, const uint &y2, const Color &color)
{
	return vlineRGBA( Img, x1, y1, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int ECImage::LineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color)
{
  return lineRGBA( Img, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int ECImage::AALineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color)
{
  return aalineRGBA( Img, x1, y1, x2, y2, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

int ECImage::CircleColor(const uint &x, const uint &y, const uint &rad, const Color &color)
{
    return circleRGBA( Img, x, y, rad, color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

ECImage* ECImage::Shadow()
{
	if(shadowed) return shadowed;

	shadowed = new ECImage;

	shadowed->NewSurface(Point2i(CASE_WIDTH, CASE_HEIGHT), SDL_HWSURFACE, false);
	shadowed->Blit(this);
	SDL_Rect r_back = {0,0,GetWidth(), GetHeight()};
	ECImage brouillard;
	brouillard.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE, GetWidth(), GetHeight(),
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
	brouillard.FillRect(r_back, brouillard.MapRGBA(0, 0, 0, 255*5/10));
	shadowed->Blit(brouillard);

	return shadowed;
}

Uint32 ECImage::GetPixel(int x, int y){
    int bpp = Img->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)Img->pixels + y * Img->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
		throw ECExcept("", "Unknow bpp!");
        return 0;   // To make gcc happy
    }
}

void ECImage::PutPixel(int x, int y, Uint32 pixel){
    int bpp = Img->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)Img->pixels + y * Img->pitch + x * bpp;

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
