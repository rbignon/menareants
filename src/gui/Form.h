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
 * For exemple, you can use a function like "void ButtonClick(void* Form, void* data)" and do :
 * <pre>
 *   Button->SetClickedFunc(&ButtonClick, 0);
 * </pre>
 * After, if you use TForm::Actions() function in your loop, when this button will be
 * selected, it will call ButtonClick() function.
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
 *      // D�fine the events
 *      Button1->SetClickedFunc(&ButtonClick, 0);
 *      MyListBox->SetSelectedFunc(&MyListBoxOnSelect, 0);
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
 *    void Button1OnClick(int mouse_x, int mouse_y);
 *    void MyListBoxOnSelect(uint selected);
 * };
 * </pre>
 *
 * --Progs
 */

/** This is a class who show a form on the screen and contain some components */
class TForm : public TObject
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

	#define ACTION_NOFOCUS       0x001
	#define ACTION_NOCLIC        0x002
	#define ACTION_NOCALL        0x004
	#define ACTION_NOKEY         0x008
	#define ACTION_NOMOUSE       (ACTION_NOCLIC|ACTION_NOFOCUS)
	/** Call this function if you want than TForm ask SDL_PollEvent */
	void Actions(uint a = 0);

	/** When you have a loop with a SDL_PollEvent() function called, use this function.
	 * You have to put the event variable.
	 * This function will set any components to Focused.
	 */
	void Actions(SDL_Event event, uint a = 0);

/* Attributs */
public:

	/** Set background picture */
	void SetBackground(ECImage *image);

/* Variables prot�g�es */
protected:

	/** Add a component */
	template<typename T>
	T* AddComponent(T* comp)
	{
		composants.push_back(comp);
		comp->SetParent(this);
		comp->Init();
		return comp;
	}

	void SetFocusOrder(bool s = true) { focus_order = s; }
	bool FocusOrder() { return focus_order; }

/* Variables priv�es */
private:
	std::vector<TComponent*> composants;
	ECImage *background;
	bool focus_order;
};

#endif /* EC_FORM_H */
