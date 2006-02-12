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

#include "Images.h"
#include "../Main.h"
#include "../Defines.h"

#ifdef WIN32
#	define PKGDATADIR
#endif

/****************************************************************************************
 *                                      ECSprite                                        *
 ****************************************************************************************/

int ECSprite::init(ECSpriteBase *base, SDL_Surface *screen)
{
  assert(base);
  assert(screen);

  mSpriteBase = base;
  if(mSpriteBase->mBuilt)
  {
    if(mSpriteBase->mNumframes>1) mAnimating=1;
    mBackreplacement = SDL_DisplayFormat(mSpriteBase->mAnim[0].Img);
  }
  mScreen = screen;
  return 0;
}

void ECSprite::clearBG()
{
  if(mDrawn==1)
  {
    SDL_Rect dest;
    dest.x = mOldX;
    dest.y = mOldY;
    dest.w = mSpriteBase->mW;
    dest.h = mSpriteBase->mH;
    SDL_BlitSurface(mBackreplacement, NULL, mScreen, &dest);
  }
}

void ECSprite::updateBG()
{
  SDL_Rect srcrect;
  srcrect.w = mSpriteBase->mW;
  srcrect.h = mSpriteBase->mH;
  srcrect.x = mX;
  srcrect.y = mY;
  mOldX=mX;mOldY=mY;
  SDL_BlitSurface(mScreen, &srcrect, mBackreplacement, NULL);
}

void ECSprite::draw()
{
  if(mAnimating == 1)
  {
    if(mLastupdate+mSpriteBase->mAnim[mFrame].pause*mSpeed<SDL_GetTicks())
    {
      mFrame++;
      if(mFrame>mSpriteBase->mNumframes-1) mFrame=0;
      mLastupdate = SDL_GetTicks();
    }
  }

  if(mDrawn==0) mDrawn=1;

  SDL_Rect dest;
  dest.x = mX; dest.y = mY;
  SDL_BlitSurface(mSpriteBase->mAnim[mFrame].Img, NULL, mScreen, &dest);
}

int ECSprite::GetWidth()
{
	return (mSpriteBase ? mSpriteBase->mW : 0);
}
int ECSprite::GetHeight()
{
	return (mSpriteBase ? mSpriteBase->mH : 0);
}

/****************************************************************************************
 *                                      ECSpriteBase                                    *
 ****************************************************************************************/

ECSpriteBase::ECSpriteBase(char *dir)
{
	mAnim = 0;
	mBuilt = 0;
	mNumframes = 0;
	mW = 0;
	mH = 0;

	init(dir);
}

int ECSpriteBase::init(char *dir)
{
  char buffer[255];
  char filename[255];
  char name[255];
  int pause=0, r=0, g=0, b=0;
  FILE *fp;

  sprintf(filename, PKGDATADIR_ANIMS "%s/info", dir);

  if((fp=fopen(filename, "r")) == NULL)
  {
    printf("ERROR opening file %s\n\n", filename);
    return -1;
  }

  fgets(buffer, 255, fp);
  sscanf(buffer, "FILES: %d", &mNumframes);
  mAnim = new ECImage[mNumframes];

  mBuilt = 1;

  for(int count=0;!feof(fp) && count<mNumframes;)
  {
    fgets(buffer, 255, fp);
    if(buffer[0] != '#' && buffer[0] != '\r' && buffer[0] != '\0' && buffer[0] != '\n' && strlen(buffer) != 0)
    {
      sscanf(buffer, "%s %d %d %d %d", name, &pause, &r, &g, &b);
      sprintf(filename, PKGDATADIR_ANIMS "%s/%s", dir, name);
      SDL_Surface *temp;
      if((temp = IMG_Load(filename)) == NULL) { printf("die (%s)\n", filename); return -1; }
      if(r >= 0) SDL_SetColorKey(temp, SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, r, g, b));
      mAnim[count].Img = SDL_DisplayFormat(temp);
      SDL_FreeSurface(temp);

      mAnim[count].pause = pause;
      if(!mW) mW = mAnim[count].Img->w;
      if(!mH) mH = mAnim[count].Img->h;

      count++;
    }
  }
  fclose(fp);
  return 0;
}



/****************************************************************************************
 *                                      ECImage                                         *
 ****************************************************************************************/

ECImage::ECImage(char* fichier)
{
	Load(fichier);
}

ECImage::~ECImage()
{
	SDL_FreeSurface(Img);
}

void ECImage::Load(char *fichier)
{
	//SDL_Surface *tmp;
	Img = IMG_Load(fichier);
	//Img = SDL_DisplayFormat(tmp);
	//SDL_FreeSurface(Img);
}

void ECImage::Draw(int x, int y)
{
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_BlitSurface(Img, NULL, app.sdlwindow, &dest);
}

void ECImage::Draw(int x, int y, int w, int h, int x2, int y2)
{
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_Rect dest2;
  dest2.x = x2;
  dest2.y = y2;
  dest2.w = w;
  dest2.h = h;
  SDL_BlitSurface(Img, &dest2, app.sdlwindow, &dest);
}

void ECImage::Draw()
{
  Draw(0, 0);
}

