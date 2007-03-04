/* src/gui/Component.h - Header of Component.cpp
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

#ifndef EC_COMPONENT_H
#define EC_COMPONENT_H

#include <vector>
#include <SDL_keyboard.h>
#include <string>
#include "Object.h"
#include "tools/Rectangle.h"

#define MyComponent(x) do { (x)->SetParent(this); (x)->SetWindow(Window()); (x)->Init(); } while(0)

typedef unsigned int   uint;
typedef void (*TOnClickFunction) (TObject* Object, void* Data);
typedef void (*TOnClickPosFunction) (TObject* Object, const Point2i& position);
typedef void (*TOnMouseOnFunction) (TObject* Object, void* Data);

/********************************************************************************************
 *                               TComponent                                                 *
 ********************************************************************************************/
/** Base of components. */
class TComponent : public TObject, public Rectanglei
{
/* Constructeur/Deconstructeur */
public:

	/** Default constructor, set x, y, h and w to 0 and \a visible and \a enabled to true. */
	TComponent(ECImage* w = 0)
		: TObject(w), Tag(0), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  want_redraw(false), always_redraw(false)
	{}

	/** Constructor with position
	 * @param _x x position
	 * @param _y y position
	 * @param w Window
	 */
	TComponent(int _x, int _y, ECImage* w = 0)
		: TObject(w), Rectanglei(_x, _y, 0, 0), Tag(0), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  want_redraw(false), always_redraw(false)
	{}

	/** Constructor with position and size
	 * @param _x x position
	 * @param _y y position
	 * @param _w width of component
	 * @param _h heigh of component
	 * @param w Window
	 */
	TComponent(int _x, int _y, uint _w, uint _h, ECImage* w = 0)
		: TObject(w), Rectanglei(_x, _y, _w, _h), Tag(0), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  want_redraw(false), always_redraw(false)
	{}

	/** Constructor with position and size
	 * @param _rect Position and size
	 * @param w Window
	 */
	TComponent(const Rectanglei& _rect, ECImage* w = 0)
		: TObject(w), Rectanglei(_rect), Tag(0), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  want_redraw(false), always_redraw(false)
	{}

	virtual ~TComponent() {}

/* Méthodes */
public:

	/** Draw the object. */
	virtual void Draw(const Point2i&) = 0;

	/** Initialization. */
	virtual void Init() = 0;

/* Attributs */
public:

	virtual void SetHeight (int sizeY);
	virtual void SetWidth (int sizeX);

	/* Visibilité */
	bool Visible() const { return visible; }              /**< Is this object visible ? */
	void Show() { SetVisible(true); }                     /**< Set visible to true */
	void Hide() { SetVisible(false); }                    /**< Set visible to false */
	void SetVisible(bool b = true) { SetWantRedraw(); visible = b; }

	virtual void SetXY(int x, int y) { Rectanglei::SetXY(x,y); SetWantRedraw(); }

	void SetSizePosition(const Rectanglei& rect)
	{
		SetXY(rect.X(), rect.Y());
		SetHeight(rect.Height());
		SetWidth(rect.Width());
	}

	bool Enabled() const { return enabled; }              /**< Is this object enabled ? */
	virtual void SetEnabled(bool _en = true)              /**< Set or unset this objet as enabled */
		{ SetWantRedraw(); enabled = _en; }

	/* Le composant a le focus ? */
	bool Focused() const { return focus; }
	virtual void SetFocus() { focus = true; SetWantRedraw(); }
	virtual void DelFocus() { focus = false; SetWantRedraw(); }

	virtual inline bool Mouse (const Point2i& pos) const { return (visible && Contains(pos)); }

	/* Pas terrible, mais pour éviter une dépendance, on ne met pas SDL_BUTTON_LEFT mais sa valeur qui est 1.
	 * On considère donc que par défaut le boutton cliqué est le GAUCHE, et donc pour que le test réussisse il faut justement
	 * que le bouton soit le GAUCHE.
	 */
	virtual bool Test (const Point2i& pos, int button = 1) const { return (Mouse(pos) && Enabled() && button == 1); }
	virtual bool Clic (const Point2i& pos, int button) { return Test(pos, button); }
	virtual void ClicUp (const Point2i& pos, int button) {}

	/** Called when user press a key... */
	virtual void PressKey(SDL_keysym) { return; }

	/** Call back when user clic on component */
	void SetOnClick(TOnClickFunction c, void* param) { on_click_func = c; on_click_param = param; }
	TOnClickFunction OnClick() const { return on_click_func; }
	void* OnClickParam() const { return on_click_param; }

	/** Call back when user clic on component. We give mouse position too. */
	void SetOnClickPos(TOnClickPosFunction c) { on_click_pos_func = c;  }
	TOnClickPosFunction OnClickPos() const { return on_click_pos_func; }

	/** Call back when user's mouse is on this component */
	void SetOnMouseOn(TOnMouseOnFunction m, void* param) { on_mouse_on_func = m; on_mouse_on_param = param; }
	TOnMouseOnFunction OnMouseOn() const { return on_mouse_on_func; }
	void* OnMouseOnParam() const { return on_mouse_on_param; }

	/** This component is always focused. */
	void SetForceFocus(bool b = true) { force_focus = b; }
	bool ForceFocus() const { return force_focus; }

	template<typename T>
	void SetHint(T h) { hint = h; }
	std::string& Hint() { return hint; }
	bool HaveHint() const { return !hint.empty(); }

	virtual bool IsHint(const Point2i& pos) const { return HaveHint() && Mouse(pos); }

	int Tag;

	void SetWantRedraw(bool b = true) { want_redraw = b; }
	bool WantRedraw() const { return (want_redraw || always_redraw); }

	void SetAlwaysRedraw(bool b = true) { always_redraw = b; }
	bool AlwaysRedraw() const { return always_redraw; }

	virtual bool RedrawBackground() const { return true; }

/* Variables privées */
private:
	bool visible;
	bool enabled;
	bool focus;
	bool force_focus;
	TOnClickFunction on_click_func;
	void* on_click_param;
	TOnClickPosFunction on_click_pos_func;
	TOnMouseOnFunction on_mouse_on_func;
	void* on_mouse_on_param;
	std::string hint;
	bool want_redraw;
	bool always_redraw;
};
typedef std::vector<TComponent*> ComponentVector;

/********************************************************************************************
 *                                 TList                                                    *
 ********************************************************************************************/
/** @page TList_usage Usage of TList
 *
 * This component is usefull when we have to show a list of components in a TForm.
 * To do this action automaticaly (with TForm::Update() and without redefine this function),
 * we have to use TList.
 *
 * TList is a contener of TComponents*. You can set a single real component or create
 * a special component who contain some components too.
 *
 * Usage :
 *
 * Add a TList in the form. You can add or remove dynamically a "list" of components.
 * This components are a derived of TComponent called for example TElement. You have
 * to define Draw() function to show yourself TElement components. TElement is
 * considered as a kind of form.
 *
 * Schema is :
 * <pre>
 *     TForm
 *     |- TButton
 *     |- TList
 *     |  |- TElement
 *     |  |  |- TSpinEdit
 *     |  |  `- TLabel
 *     |  `- TElement
 *     |     |- TSpinEdit
 *     |     `- TLabel
 *     |- TMemo
 *     `- TEdit
 * </pre>
 *
 * Note that all components in list have their position setted automaticaly.
 * Heigh is automaticaly setted when you add or remove a component and use heigh
 * of each components.
 * Width is width of biggest component.
 *
 * --Progs
 */
/** This is a particular component who show a list of components in a TForm */
class TList : public TComponent
{
/* Constructeur/Deconstructeur */
public:

	/** Constructor of TList.
	 * @param _x this is x position of first component in list
	 * @param _y this is y position of first component in list
	 */
	TList(int _x, int _y);

	~TList();

/* Méthodes */
public:

	/** Add a component in the list */
	template<typename T>
	void AddLine(T* c)
	{
		list.push_back(c);
		c->SetParent(this);
		c->SetWindow(Window());
		c->SetXY(X(), Y() + Height());
		SetHeight(Height() + c->Height());
		if(c->Width() > Width()) SetWidth(c->Width());
		c->Init();
		SetWantRedraw();
	}

	void Clear();

	/** Remove a component from list
	 * \warning This will use \a delete on TComponent !!
	 */
	bool RemoveLine(TComponent *c, bool use_delete = false);

	/** Draw all components in list */
	void Draw(const Point2i&);

	/** Initialization not requiered... But it is a virtual function so i have to implemente this  */
	void Init() {}

	virtual bool Clic (const Point2i&, int button);

	virtual void ClicUp (const Point2i&, int button);

	virtual void DelFocus();

/* Attributs */
public:

	void SetXY (int x, int y); /** Reimplementation to affect all components in the list */

	/** Get list of components as a vector */
	ComponentVector GetList() const { return list; }

	virtual bool IsHint(const Point2i& pos) const { return HaveHint(); }

/* Variables privées */
private:

	/** Set \a x for all components in list, and reset height of TList */
	void Rebuild();

	ComponentVector list;
};

#endif /* EC_COMPONENT_H */
