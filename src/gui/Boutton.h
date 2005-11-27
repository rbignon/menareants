/* src/gui/Boutton.h - Header of Boutton.cpp
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
 *
 * $Id$
 */

#ifndef EC_BOUTTON_H
#define EC_BOUTTON_H

class ECSprite;

class Button
{
protected:
  unsigned int m_x, m_width, m_y, m_height;

  ECSprite *image;

  bool enabled;

public:
  Button();
  Button (unsigned int x, unsigned int y, unsigned int w, unsigned int h);
  virtual ~Button();
  void SetPos(unsigned int x, unsigned int y);
  void SetSize (unsigned int larg, unsigned int haut);

  void SetImage (ECSprite *image);

  bool Test (unsigned int souris_x, unsigned int souris_y);
  virtual void Draw (unsigned int souris_x, unsigned int souris_y);
  unsigned int GetX() const;
  unsigned int GetY() const;
  unsigned int GetWidth() const;
  unsigned int GetHeight() const;

  bool Enabled() { return enabled; }
  void SetEnabled(bool _en) { enabled = _en; }

protected:
  void DrawImage (unsigned int souris_x, unsigned int souris_y);
};


#endif /* EC_BOUTTON_H */
