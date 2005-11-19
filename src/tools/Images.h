/* src/tools/Images.h - Header of Images.cpp
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

#ifndef EC_IMAGES_H
#define EC_IMAGES_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

class ECSpriteBase;

class ECImage;

class ECSprite
{
/* Constructeur/Deconstructeur */
public:
	ECSprite() {}
	~ECSprite() {};

/* Methodes */
public:

	/* Initialisation */
	int init(ECSpriteBase *base, SDL_Surface *screen);

	/* Dessine l'animation */
	void draw();

	/* Vide le background */
	void clearBG();

	/* Met à jour le background */
	void updateBG();

/* Attributs */
public:

	/* Paramètre ou retourne le nombre de frames */
	void setFrame(int nr) { mFrame = nr; }
	int getFrame() { return mFrame; }

	/* Paramètre ou retourne la vitesse */
	void setSpeed(float nr) { mSpeed = nr; }
	float getSpeed() { return mSpeed; }

	/* Active/Desactive l'animation */
	void toggleAnim() { mAnimating = !mAnimating; }
	void startAnim() { mAnimating = 1; }
	void stopAnim() { mAnimating = 0; }

	/* Repasse à la première frame */
	void rewind() { mFrame = 0; }

	/* Fonctions pour changer les coordonnées */
	void xadd(int nr) { mX+=nr; }
	void yadd(int nr) { mY+=nr; }
	void xset(int nr) { mX=nr; }
	void yset(int nr) { mY=nr; }
	void set(int xx, int yy) { mX=xx; mY=yy; }

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

	/* Initialisation.
	 * Ouvre dans un répertoire le fichier info dans lequel est inscrit les informations
	 * pour l'animation (images, frequence, etc..)
	 */
	int init(char *dir);

/* Variables publiques */
public:
	ECImage *mAnim;
	int mBuilt, mNumframes, mW, mH;
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

	/* Charge l'image à partir d'un fichier */
	void Load(char *fichier);

	/* Dessin à la taille originale à la position (x,y) */
	void Draw(int x, int y);

	/* Dessin avec une taille différente à la position (x,y) */
	void Draw(int x, int y, int w, int h, int x2, int y2);

	/* Dessine en 0x0 (background) */
	void Draw();

/* Variables publiques */
public:
	SDL_Surface* Img;
	bool pause;
};

#endif /* EC_IMAGES_H */
