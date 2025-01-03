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
#include <bitset>

/** @page TForm_usage Usage of TForm
 *
 * You have to define a derived class from TForm. Put in components as PUBLICS POINTERS.
 * It is necessary to initialize all variables in the constructor as this :
 * <pre>VarComp = AddComponent(new TComponent());</pre>
 * AddComponent function add the component in list and call his Init() function.
 *
 * After, when you use this Form, you have to create the object.
 * You must use a no-end-loop where you call TForm::Update() function.
 * It is possible to each components to define some functions to execute when there is an action.
 * For exemple, you can use a function like "void ButtonClick(void* Form, void* data)" and do :
 * <pre>
 *   Button->SetOnClick(&ButtonClick, 0);
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
 *    TForm1(ECImage* screen)
 *      : TForm(screen)
 *    {
 *      // Add components
 *      Button1 =   AddComponent(new TButton(500,350,100,49));
 *      MyListBox = AddComponent(new TListBox(300,200,200,300));
 *      Blah =      AddComponent(new TMemo(75,325,300,200,30));
 *      // Défine the events
 *      Button1->SetOnClick(&ButtonClick, 0);
 *      MyListBox->SetOnSelect(&MyListBoxOnSelect, 0);
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
 * To use a TForm, use a function like this :
 *
 * <pre>
 * void MyForm()
 * {
 *   TForm1* Form1 = new TForm1(Window());
 *   Form1->Run(!want_quit());
 *   return;
 * }
 * </pre>
 *
 * --Progs
 */

#include "Memo.h"

class SDL_mutex;

#define FORM_RUN(form, condition) do { (form)->Actions(); (form)->Update(); } while((condition))

/** This is a class who show a form on the screen and contain some components */
class TForm : public TObject
{
/* Constructeur/Destructeur */
public:

	TForm(ECImage* window);
	virtual ~TForm();

/* Methodes */
public:

	/** Draw all components
	 * @param flip If true, function calls SDL_Flip() and draw directly with SDL
	 */
	void Update(bool flip = true);

	void Draw();

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

	void Run(bool *(func)());
	void Run(uint a = 0);

/* Evenements */
protected:

	virtual void OnClic(const Point2i& mouse, int button, bool& stop) {}
	virtual void OnClicUp(const Point2i& mouse, int button) {}
	virtual void BeforeDraw() {}
	virtual void AfterDraw() {}
	virtual void OnMouseMotion(const Point2i& mouse) {}
	virtual void OnKeyDown(SDL_keysym key) {}
	virtual void OnKeyUp(SDL_keysym key) {}
	virtual void LockedBeforeDraw() {}

/* Attributs */
public:

	/** Set background picture */
	void SetBackground(ECImage *image);

	int Width() const { return Window()->GetWidth(); }
	int Height() const { return Window()->GetHeight(); }

	void SetMutex(SDL_mutex* m) { mutex = m; }

	void LockScreen() const;
	void UnlockScreen() const;

	bool IsPressed(SDLKey i) const
	{
		Uint8 *keystate = SDL_GetKeyState(NULL);
		return keystate[i];
	}

	void SetMaxFPS(int mps) { max_fps = mps; }
	int MaxFPS() const { return max_fps; }

	bool MustRedraw() const { return must_redraw; }
	void SetMustRedraw(bool b = true) { must_redraw = b; }

	bool WantQuit() const { return want_quit; }
	void SetWantQuit(bool b = true) { want_quit = b; }

	static std::string Message;

/* Variables protégées */
protected:

	/** Add a component */
	template<typename T>
	T* AddComponent(T* comp)
	{
		composants.push_back(comp);
		comp->SetParent(this);
		comp->SetWindow(Window());
		comp->Init();
		return comp;
	}

	void RemoveComponent(TComponent* comp);

	void SetFocusOrder(bool s = true) { focus_order = s; }
	bool FocusOrder() const { return focus_order; }

	void Clear();

	bool want_quit;

/* Variables privées */
private:
	std::vector<TComponent*> composants;
	ECImage *background;
	bool focus_order;
	TMemo Hint;
	SDL_mutex* mutex;
	uint max_fps;
	bool must_redraw;
	Point2i lastmpos;
	uint last_move_time;
};

#endif /* EC_FORM_H */
