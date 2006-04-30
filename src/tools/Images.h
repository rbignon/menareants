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

class ECSpriteBase;

class ECImage;

void ChangePixelColor(ECImage* surf, SDL_Color last_color, SDL_Color new_color);
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
	ECSprite(ECSpriteBase* _base, SDL_Surface* _screen) { init(_base, _screen); }
	~ECSprite() {};

/* Methodes */
public:

	/** Initialisation */
	int init(ECSpriteBase *base, SDL_Surface *screen);

	/** Dessine l'animation */
	void draw();

	/** Vide le background */
	void clearBG();

	/** Met à jour le background */
	void updateBG();

/* Attributs */
public:

	/** Paramètre ou retourne le nombre de frames */
	void setFrame(int nr) { mFrame = nr; }
	int getFrame() { return mFrame; }

	/** Paramètre ou retourne la vitesse */
	void setSpeed(float nr) { mSpeed = nr; }
	float getSpeed() { return mSpeed; }

	/** Active/Desactive l'animation */
	void toggleAnim() { mAnimating = !mAnimating; }
	void startAnim() { mAnimating = 1; }
	void stopAnim() { mAnimating = 0; }
	void SetAnim(bool a) { mAnimating = a ? 1 : 0; }

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

/* Variables privées */
private:
	int mFrame;
	int mX, mY, mOldX, mOldY;
	int mAnimating;
	int mDrawn;
	float mSpeed;
	long mLastupdate;
	ECSpriteBase *mSpriteBase;
	SDL_Surface *mBackreplacement;
	SDL_Surface *mScreen;
};

class ECSpriteBase
{
/* Methodes */
public:

	/** Initialisation.
	 * Ouvre dans un répertoire le fichier info dans lequel est inscrit les informations
	 * pour l'animation (images, frequence, etc..)
	 */
	int init(char *dir);

	ECSpriteBase(char *dir);
	~ECSpriteBase() {}

/* Variables publiques */
public:
	ECImage *mAnim;
	int mBuilt, mNumframes, mW, mH;
	bool animation;
};

class ECImage
{
/* Constructeur/Deconstructeur */
public:
	ECImage(SDL_Surface* _Img) : Img(_Img) {}
	ECImage(char* fichier);
	ECImage() : Img(NULL) {}

	~ECImage();

/* Methodes */
public:

	/** Charge l'image à partir d'un fichier */
	void Load(char *fichier);

	/** Dessin à la taille originale à la position (x,y) */
	void Draw(int x, int y);

	/** Dessin avec une taille différente à la position (x,y) */
	void Draw(int x, int y, int w, int h, int x2, int y2);

	/** Dessine en 0x0 (background) */
	void Draw();

	/** Obtenir la largeur */
	unsigned int GetWidth() { return (Img ? Img->w : 0); }

	/** Obtenir la hauteur */
	unsigned int GetHeight() { return (Img ? Img->h : 0); }

	void SetColorKey(unsigned int, unsigned int, unsigned int);

	ECImage &operator=(const ECImage &src);

	void Free();

	void SetImage(SDL_Surface* i) { Free(); Img = i; }

/* Variables publiques */
public:
	SDL_Surface* Img;
	int pause;
};

#endif /* EC_IMAGES_H */
