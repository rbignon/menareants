/* src/gui/SpinEdit.h - Header of SpinEdit.cpp
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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

#ifndef EC_SPINEDIT_H
#define EC_SPINEDIT_H

#define SPINEDIT_HEIGHT 15
#include "Component.h"
#include "Boutton.h"
#include "Label.h"
#include <string>

class Color;
class Font;

/** This component show an edit who you can choose an integer in an interval */
class TSpinEdit : public TComponent
{
/* Constructeur/Destructeur */
public:

	/** Constructor
	 * @param font font
	 * @param label label of component.
	 * @param _x x position of component.
	 * @param _y y position of component.
	 * @param _width width of component.
	 * @param _min min value that component can take.
	 * @param _max max value that component can take.
	 * @param _step value added or removed when user clicks on one of buttons.
	 * @param _defvalue this is default value that component has when it is created.
	 */
	TSpinEdit(Font *font, std::string label, int _x, int _y, uint _width, int _min, int _max, uint _step = 1,
	          int _defvalue = 0);

/* Methodes */
public:

	virtual void Init();

	void Draw (const Point2i&);

	bool Clic (const Point2i&, int button);

	void SetColorFont(Color new_color, Font* new_font);

/* Attributs */
public:

	/** Value of spinedit.
	 * @return true if value is changed
	 */
	virtual bool SetValue(int _value, bool first = false);
	int Value() { return value; }

	void SetMax(int _max);                                        /**< Set maximal value */
	void SetMin(int _min);                                        /**< Set minimal value */

	virtual void SetXY (int _x, int _y);                          /**< Set \a x and \a y positions */

	/* Valeurs interdites */
	void AddBadValue(int i);                                      /**< Add a bad value */
	void DelBadValue(int i);                                      /**< Remove a bad value */

/* Variables privées */
protected:
	TButton m_plus, m_minus;
	TLabel txt_label, txt_value;
	int value, min, max;
	uint step;
	uint visible_len;
	std::string label;

	std::vector<int> bad_values;

	/** This function is used by Clic() to check in bad_values */
	bool ChangeValueByClick(bool up);

	Color color;
	Font *font;
};

#endif /* EC_SPINEDIT_H */
