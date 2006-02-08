/* src/gui/Form.h - Header of Form.cpp
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

#ifndef EC_FORM_H
#define EC_FORM_H

#include "Component.h"
#include "tools/Images.h"
#include <vector>

/** @page TForm_usage Usage of TForm
 *
 * You have to define a derived class from TForm. Put in components as PUBLICS POINTERS.
 * It is necessary to initialize all variables in the constructor as this :
 * <pre>VarComp = AddComponent(new TComponent());</pre>
 * AddComponent function add the component in list and call his Init() function.
 * You have to destroy components too in the destructor with \a delete.c delete.
 *
 * After, when you use this Form, you have to create the object.
 * You must use a no-end-loop where you call TForm::Update() function.
 * It is possible to each components to define some functions to execute when there is an action.
 *
 * This is an example of a derived class from TForm :
 *
 * <pre>
 * class TForm1 : public TForm
 * {
 * // Constructor/Deconstructor
 * public:
 *
 *    // Note: you have to implemente this two functions outside of class definition.
 *    TForm1()
 *    {
 *      // Add components
 *      Button1 =   AddComponent(new TButton(500,350,100,49));
 *      MyListBox = AddComponent(new TListBox(300,200,200,300));
 *      Blah =      AddComponent(new TMemo(75,325,300,200,30));
 *      // Défine the events
 *      Button1->OnClick =    Button1OnClick;
 *      MyListBox->OnSelect = MyListBoxOnSelect;
 *    }
 *    ~TForm1()
 *    {
 *      delete Button1;
 *      delete MyListBox;
 *      delete Blah;
 *    }
 *
 * // Components
 * public:
 *
 *    TButton      *Button1;
 *    TListBox     *MyListBox;
 *    TMemo        *Blah;
 *
 * // Events
 * public:
 *    void Button1OnClick();
 *    void MyListBoxOnSelect(uint selected);
 * };
 * </pre>
 *
 * --Progs
 */

/** This is a class who show a form on the screen and contain some components */
class TForm
{
/* Constructeur/Destructeur */
public:

	TForm();
	virtual ~TForm() {}

/* Methodes */
public:

	/* Dessine chaques composants */
	void Update(); /**< Draw all components */
	void Update(bool flip); /**< Draw all components @param flip If true, function calls SDL_Flip() */
	void Update(int x, int y); /**< Draw all components @param x x position of mouse @param y y position of mouse */
	/** Draw all components
	 * @param x x position of mouse
	 * @param y y position of mouse
	 * @param flip If true, function calls SDL_Flip() and draw directly with SDL
	 */
	void Update(int x, int y, bool flip);

/* Attributs */
public:

	/** Set background picture */
	void SetBackground(ECImage *image);

/* Variables protégées */
protected:

	ECImage *background;

	/** Add a component */
	template<typename T>
	T* AddComponent(T* comp)
	{
		composants.push_back(comp);
		comp->Init();
		return comp;
	}

/* Variables privées */
private:
	std::vector<TComponent*> composants;
};

#endif /* EC_FORM_H */
