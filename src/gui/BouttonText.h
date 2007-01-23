/* src/gui/BouttonText.h - Header of BouttonText.cpp
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
 *
 * $Id$
 */

#ifndef EC_BOUTTONTEXT_H
#define EC_BOUTTONTEXT_H

#include "Boutton.h"
#include "Label.h"
#include <string>
#include <SDL.h>

class Font;

class TButtonText : public TButton
{
private:
  TLabel label;

  Font *font;

public:
  TButtonText();
  TButtonText (int x, int y, unsigned int w, unsigned int h, const std::string &text, Font* font);

  void Init();

  virtual void SetImage (ECSprite *image);

  virtual void Draw (const Point2i&);
  void SetText (const std::string &text);
  void SetEnabled(bool b = true);
  virtual void SetXY(int, int);

  std::string GetText() const;
};

#endif /* EC_BOUTTONTEXT_H */
