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

#ifndef EC_IMAGES_H
#define EC_IMAGES_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string>
#include "Defines.h"


class ECSpriteBase;
class Color;
class ECImage;

void ChangePixelColor(ECImage* surf, Color last_color, Color new_color);
Uint32 getpixel(SDL_Surface * surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void DrawRect(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color);
#define EstTransparent(a)       ( (a) != 255 )
SDL_Surface* CreateRGBSurface (int width, int height, Uint32 flags);
SDL_Surface* CreateRGBASurface (int width, int height, Uint32 flags);
void DrawLine(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color);
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

	/** Param�tre ou retourne le nombre de frames */
	void SetFrame(uint nr);
	uint Frame() { return mFrame; }
	uint NbFrames() const;

	/** Param�tre ou retourne la vitesse */
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

	/** Repasse � la premi�re frame */
	void rewind() { mFrame = 0; }

	/* Fonctions pour changer les coordonn�es */
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

	void ChangeColor(Color first, Color to);

	ECImage* Window() const { return mScreen; }

/* Variables priv�es */
private:
	uint mFrame;
	int mX, mY, mOldX, mOldY;
	bool mAnimating;
	int mDrawn;
	float mSpeed;
	long mLastupdate;
	bool order, repeat;
	ECSpriteBase *mSpriteBase;
	ECImage *mScreen;
};

class ECSpriteBase
{
/* Methodes */
public:

	/** Initialisation.
	 * Ouvre dans un r�pertoire le fichier info dans lequel est inscrit les informations
	 * pour l'animation (images, frequence, etc..)
	 */
	int init(const char *dir);

	void ChangeColor(Color from, Color to);

	ECSpriteBase(const char *dir);
	~ECSpriteBase();

	ECImage* First() const;

/* Variables publiques */
public:
	ECImage *mAnim;
	int mBuilt, mW, mH;
	uint mNumframes;
	bool animation;
	std::string path;
};

class ECImage
{
/* Constructeur/Deconstructeur */
public:
	ECImage(SDL_Surface* _Img) : Img(_Img), pause(0), autofree(true) {}
	ECImage(char* fichier);
	ECImage() : Img(NULL), pause(0), autofree(true) {}

	~ECImage();

/* Methodes */
public:

	/** Charge l'image � partir d'un fichier */
	void Load(char *fichier);

	/** Dessin � la taille originale � la position (x,y) */
	void Draw(int x, int y);

	/** Dessin avec une taille diff�rente � la position (x,y) */
	void Draw(int x, int y, int w, int h, int x2, int y2);

	/** Dessine en 0x0 (background) */
	void Draw();

	/** Obtenir la largeur */
	unsigned int GetWidth() { return (Img ? Img->w : 0); }

	/** Obtenir la hauteur */
	unsigned int GetHeight() { return (Img ? Img->h : 0); }

	void SetColorKey(unsigned int, unsigned int, unsigned int);

	int Blit(const ECImage* src, SDL_Rect *srcRect, SDL_Rect *dstRect);
	int Blit(const ECImage* src, SDL_Rect *dstRect);
	int Blit(const ECImage* src);
	int Blit(const ECImage& src, SDL_Rect *srcRect, SDL_Rect *dstRect);
	int Blit(const ECImage& src, SDL_Rect *dstRect);
	int Blit(const ECImage& src);

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

/* Attributs */
public:

	void SetImage(SDL_Surface* i, bool use_delete = true) { if(use_delete) Free(); Img = i; }

	SDL_Surface* Surface() const { return Img; }

	bool IsNull() const { return !Img; }

	void SetAutoFree(bool b = true) { autofree = b; }

/* Variables publiques */
public:
	SDL_Surface* Img;
	int pause;
	bool autofree;
};

#endif /* EC_IMAGES_H */
