/* src/tools/Images.h - Header of Images.cpp
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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

#ifndef EC_IMAGES_H
#define EC_IMAGES_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include "Defines.h"
#include "tools/Rectangle.h"

class ECSpriteBase;
class Color;
class ECImage;

void ChangePixelColor(ECImage* surf, const Color& last_color, const Color& new_color);
Uint32 getpixel(SDL_Surface * surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void DrawRect(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color);
#define EstTransparent(a)       ( (a) != 255 )
SDL_Surface* CreateRGBSurface (int width, int height, Uint32 flags);
SDL_Surface* CreateRGBASurface (int width, int height, Uint32 flags);
void DrawLine(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color);
void DrawLargeLine(SDL_Surface* screen, int x1, int y1, int x2, int y2, Uint32 color);
#define sgn(x) ((x<0)?-1:((x>0)?1:0))
#define SLOCK(surface) \
    do { \
        if(SDL_MUSTLOCK(surface)) \
	    SDL_LockSurface(surface); \
    } while(0)

#define SUNLOCK(surface) \
    do { \
        if(SDL_MUSTLOCK(surface)) \
	    SDL_UnlockSurface(surface); \
    } while(0)

class ECSprite
{
/* Constructeur/Deconstructeur */
public:
	ECSprite(ECSpriteBase* _base, ECImage* _screen) { init(_base, _screen); }
	~ECSprite();

/* Methodes */
public:

	/** Initialisation */
	int init(ECSpriteBase *base, ECImage *screen);

	/** Dessine l'animation */
	void draw();

/* Attributs */
public:

	/** Paramètre ou retourne le nombre de frames */
	void SetFrame(uint nr);
	uint Frame() { return mFrame; }
	uint NbFrames() const;

	/** Paramètre ou retourne la vitesse */
	void setSpeed(float nr) { mSpeed = nr; }
	float getSpeed() { return mSpeed; }

	/** Active/Desactive l'animation */
	void toggleAnim() { mAnimating = !mAnimating; }
	void startAnim() { mAnimating = true; }
	void stopAnim() { mAnimating = false; }
	void SetAnim(bool a) { mAnimating = a; }
	bool Anim() const;

	bool Order() const { return order; }
	void SetOrder(bool b) { order = b; }

	bool Repeat() const { return repeat; }
	void SetRepeat(bool b = true) { repeat = b; }

	/** Repasse à la première frame */
	void rewind() { mFrame = 0; }

	/* Fonctions pour changer les coordonnées */
	void xadd(int nr) { mX+=nr; }
	void yadd(int nr) { mY+=nr; }
	void xset(int nr) { mX=nr; }
	void yset(int nr) { mY=nr; }
	void set(int xx, int yy) { mX=xx; mY=yy; }

	int X() { return mX; }
	int Y() { return mY; }
	int GetWidth();                             /**< Fonction pour la largeur */
	int GetHeight();                            /**< Fonction pour la hauteur */

	ECImage* First() const;

	void ChangeColor(const Color& first, const Color& to);
	void Gray2Color(const Color& to);

	void RotoZoom(double angle, double zoomx, double zoomy, bool smooth);
	void Zoom(double zoomx, double zoomy, bool smooth);

	ECImage* Window() const { return mScreen; }
	ECSpriteBase* SpriteBase() const { return mSpriteBase; }

	void CopySpriteBase();

/* Variables privées */
private:
	uint mFrame;
	int mX, mY, mOldX, mOldY;
	bool mAnimating;
	int mDrawn;
	float mSpeed;
	long mLastupdate;
	bool order, repeat;
	ECSpriteBase *mSpriteBase;
	bool autofree;
	ECImage *mScreen;
};

class ECSpriteBase
{
/* Methodes */
public:

	/** Initialisation.
	 * Ouvre dans un répertoire le fichier info dans lequel est inscrit les informations
	 * pour l'animation (images, frequence, etc..)
	 */
	int init(const char *dir);

	void ChangeColor(const Color& from, const Color& to);
	void Gray2Color(const Color& to);

	void RotoZoom(double angle, double zoomx, double zoomy, bool smooth);
	void Zoom(double zoomx, double zoomy, bool smooth);

	ECSpriteBase(const char *dir);
	ECSpriteBase();
	~ECSpriteBase();

	ECSpriteBase* Copy() const;

	ECImage* First();

	bool Alpha() const { return alpha; }

	uint NumFrames() const { return surfaces.size(); }

/* Variables publiques */
public:
	std::vector<ECImage> surfaces;
	int mBuilt;
	uint mW, mH;
	bool animation;
	std::string path;
	bool alpha;
};

class ECImage
{
/* Constructeur/Deconstructeur */
public:
	ECImage(SDL_Surface* _Img) : Img(_Img), shadowed(0), pause(0), autofree(true), alpha(false), x(0), y(0) {}
	ECImage(const ECImage&);
	ECImage(const char* fichier, bool alpha = false);
	ECImage() : Img(0), shadowed(0), pause(0), autofree(true), alpha(false), x(0), y(0) {}
	explicit ECImage(const Point2i &size, Uint32 flags, bool useAlpha = true);

	~ECImage();

/* Methodes */
public:

	/** Charge l'image à partir d'un fichier */
	void Load(const char *fichier, bool alpha = false);

	/** Dessin à la taille originale à la position (x,y) */
	void Draw(int x, int y);

	/** Dessin avec une taille différente à la position (x,y) */
	void Draw(int x, int y, int w, int h, int x2, int y2);

	/** Dessine en 0x0 (background) */
	void Draw();

	/** Obtenir la largeur */
	int GetWidth() const { return (Img ? Img->w : 0); }

	/** Obtenir la hauteur */
	int GetHeight() const { return (Img ? Img->h : 0); }

	Point2i GetSize() const { return Img ? Point2i(Img->w, Img->h) : Point2i(0,0); }

	void SetColorKey(unsigned int, unsigned int, unsigned int);

	int BoxColor(const Rectanglei &rect, const Color &color);
	int RectangleColor(const Rectanglei &rect, const Color &color, const uint &border_size = 1);
	int VlineColor(const uint &x1, const uint &y1, const uint &y2, const Color &color);
	int LineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color);
	int AALineColor(const uint &x1, const uint &x2, const uint &y1, const uint &y2, const Color &color);
	int CircleColor(const uint &x, const uint &y, const uint &rad, const Color &color);

	int Blit(const ECImage* src, SDL_Rect *srcRect, SDL_Rect *dstRect);
	int Blit(const ECImage* src, SDL_Rect *dstRect);
	int Blit(const ECImage* src, const Rectanglei &srcRect, const Point2i &dstPoint);
	int Blit(const ECImage* src, const Point2i &dstPoint);
	int Blit(const ECImage* src);
	int Blit(const ECImage& src, SDL_Rect *srcRect, SDL_Rect *dstRect);
	int Blit(const ECImage& src, SDL_Rect *dstRect);
	int Blit(const ECImage& src);
	int Blit(const ECImage& src, const Rectanglei &srcRect, const Point2i &dstPoint);
	int Blit(const ECImage& src, const Point2i &dstPoint);

	SDL_Rect GetSDLRect(const Rectanglei &r) const;
	SDL_Rect GetSDLRect(const Point2i &pt, const Point2i &pt2) const;

	ECImage &operator=(const ECImage &src);

	void Free();

	void GetRGBA(Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a);

	Uint32 MapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	Color GetColor(Uint32 color);

	Uint32 MapColor(Color color);

	void Flip();

	int Fill(Uint32 color);
	int Fill(const Color &color);
	int FillRect(SDL_Rect &dstRect, Uint32 color);
	int FillRect(SDL_Rect &dstRect, const Color color);

	void NewSurface(const Point2i& size, Uint32 flags, bool useAlpha);

	void RotoZoom(double angle, double zoomx, double zoomy, bool smooth);
	void Zoom(double zoomx, double zoomy, bool smooth);

	void Lock() { SDL_LockSurface( Img ); }
	void Unlock() { SDL_UnlockSurface( Img ); }

	Uint32 GetPixel(int x, int y);
	void PutPixel(int x, int y, Uint32 pixel);

	void Gray2Color(const Color& new_color);

/* Attributs */
public:

	void SetImage(SDL_Surface* i, bool use_delete = true) { if(use_delete) Free(); Img = i; }

	SDL_Surface* Surface() const { return Img; }

	bool IsNull() const { return !Img; }

	void SetAutoFree(bool b = true) { autofree = b; }
	void SetAlpha(bool a = true) { alpha = a; }

	ECImage* Shadow();

	void SetPause(int _p) { pause = _p; }

	inline unsigned char *GetPixels() const
	{
		return (unsigned char *) Img->pixels;
	}

/* Variables publiques */
public:
	SDL_Surface* Img;
	ECImage* shadowed;
	int pause;
	bool autofree;
	bool alpha;
	int x, y;
};

#endif /* EC_IMAGES_H */
