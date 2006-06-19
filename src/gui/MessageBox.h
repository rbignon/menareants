/* src/gui/MessageBox.h - Header of MessageBox.cpp
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

#ifndef EC_MSGBOX_H
#define EC_MSGBOX_H

#include <string>
#include <vector>
#include <SDL.h>
#include "BouttonText.h"
#include "Form.h"
#include "Edit.h"
#include "tools/Images.h"

#define MSGBOX_MAXWIDTH 50

/** This is a complex box to show a message and some buttons */
class TMessageBox : public TObject
{
#define BT_YES		0x001               /**< Yes button */
#define BT_NO		0x002               /**< No button */
#define BT_OK		0x004               /**< OK button */
#define BT_CANCEL	0x008               /**< Cancel button */
#define HAVE_EDIT	0x010               /**< Use a TEdit */

/* Constructeur/Deconstructeur */
public:

	/** Constructor of TMessageBox.
	 *
	 * @param _x this is \a horizontal position of box.
	 * @param _y this is \a vertial position of box.
	 * @param _s this is string to show.
	 * @param _b this is flags of buttons to show.
	 * @param form if there is a TForm, to continue to show it, put it in.
	 */
	TMessageBox(int _x, int _y, const char* _s, uint _b, TForm* form = 0);

	TMessageBox(const char* _s, uint _b, TForm* form = 0);

	~TMessageBox();

/* Methodes */
public:

	/** Show message, use loop and wait a result.
	 *
	 * @return It return flag of button whose is clicked.
	 */
	uint Show();

	/** Draw box */
	void Draw (uint mouse_x, uint mouse_y);

	/** To draw this box only one time */
	void Draw();

/* Attributs */
public:

	/** Set form */
	void SetForm(TForm* _form) { Form = _form; }

	std::string EditText() const { return (edit ? edit->GetString() : ""); }
	TEdit* Edit() const { return edit; }

	/** Set real back ground. */
	void SetBackGround(ECImage* _ebg) { realbg = _ebg; }

/* Variables privées */
protected:
	int x, y;
	uint w, h;
	uint b;
	uint height_string;
	std::vector<std::string> message;
	std::vector<TButtonText*> boutons;
	SDL_Surface *background;
	ECImage *realbg;
	TForm *Form;
	TEdit *edit;

	void SetButtons();
	void SetEdit();
	void Init(const char* s);
	void SetText(const char* s);
};

#endif /* EC_MSGBOX_H */
