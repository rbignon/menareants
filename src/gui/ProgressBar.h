/* src/gui/ProgressBar.h - Header of ProgressBar.cpp
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

#ifndef EC_PROGRESSBAR_H
#define EC_PROGRESSBAR_H

#include <list>
#include "gui/Component.h"
#include "tools/Images.h"
#include "tools/Color.h"

class TProgressBar : public TComponent
{
/* Structures */
private:
	typedef struct s_marqueur_t{
		Color color;
		uint val;
	} marqueur_t;

	typedef std::list<marqueur_t>::iterator marqueur_it;
	typedef std::list<marqueur_t>::const_iterator marqueur_it_const;

/* Constructeur/Destructeur */
public:

	TProgressBar(int x, int y, uint w, uint h);

/* Methodes */
public:

	void Init() {}

	/// Draw la barre de progresssion
	void Draw(const Point2i&);

/* Attributs */
public:

	/// Ajoute/supprime un marqueur
	marqueur_it AddMarqueur (long val, const Color& coul);
	void RemoveMarqueur (marqueur_it it);
	void ResetMarqueur();

	void ChangeSize();

	void SetWidth(uint);
	void SetHeight(uint);

	/// Actualisation de la valeur
	void SetValue (long val);
	long Value() const { return val; }

	/// Initialise les valeurs
	void InitVal (long val, long min, long max);

	long Max() const { return max; }

	/** Set reference value
	 * Use it after InitVal !
	 */
	void SetReferenceValue (bool use, long value=0);

	void SetBorderColor(Color color);
	void SetBackgroundColor(Color color);
	void SetValueColor(Color color);

	void SetBackground(bool b = true) { background = b; }
	bool Background() const { return background; }


/* variables privées */
private:
	Color border_color, value_color, background_color;
	ECImage image; // in order to pemit alpha blended progressbar

	long val, min, max; // Valeur
	bool m_use_ref_val; // Valeur de référence
	long m_ref_val; // Valeur de référence
	uint val_barre; // Valeur dans la barre

	uint CalculeVal (long val) const;
	uint CalculeValBarre (long val) const;

	bool background;

	std::list<marqueur_t> marqueur;
};

#endif /* EC_PROGRESSBAR_H */
