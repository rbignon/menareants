/* src/gui/Component.h - Header of Component.cpp
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

#ifndef EC_COMPONENT_H
#define EC_COMPONENT_H

#include <vector>
#include <SDL_keyboard.h>
#include <string>
#include "Object.h"

#define MyComponent(x) do { (x)->SetParent(this); (x)->SetWindow(Window()); (x)->Init(); } while(0)

typedef unsigned int   uint;
typedef void (*TOnClickFunction) (TObject* Object, void* Data);
typedef void (*TOnClickPosFunction) (TObject* Object, int x, int y);
typedef void (*TOnMouseOnFunction) (TObject* Object, void* Data);

/********************************************************************************************
 *                               TComponent                                                 *
 ********************************************************************************************/
/** Base of components. */
class TComponent : public TObject
{
/* Constructeur/Deconstructeur */
public:

	/** Default constructor, set x, y, h and w to 0 and \a visible and \a enabled to true. */
	TComponent(SDL_Surface* w = 0)
		: TObject(w), x(0), y(0), h(0), w(0), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  dynamic_hint(0)
	{}

	/** Constructor with position
	 * @param _x x position
	 * @param _y y position
	 * @param w Window
	 */
	TComponent(int _x, int _y, SDL_Surface* w = 0)
		: TObject(w), x(_x), y(_y), h(0), w(0), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  dynamic_hint(0)
	{}

	/** Constructor with position and size
	 * @param _x x position
	 * @param _y y position
	 * @param _w width of component
	 * @param _h heigh of component
	 * @param w Window
	 */
	TComponent(int _x, int _y, uint _w, uint _h, SDL_Surface* w = 0)
		: TObject(w), x(_x), y(_y), h(_h), w(_w), visible(true), enabled(true), focus(false), force_focus(false),
		  on_click_func(0), on_click_param(0), on_click_pos_func(0), on_mouse_on_func(0), on_mouse_on_param(0),
		  dynamic_hint(0)
	{}

	virtual ~TComponent() {}

/* Méthodes */
public:

	/** Draw the object. */
	virtual void Draw(int souris_x, int souris_y) = 0;

	/** Initialization. */
	virtual void Init() = 0;

/* Attributs */
public:

	/* Obtient la position, la hauteur ou la largeur */
	int X() const;                                        /**< Get \a x position. */
	int Y() const;                                        /**< Get \a y position. */
	unsigned int Width() const;                           /**< Get \a width position. */
	unsigned int Height() const;                          /**< Get \a height position. */

	/* Définie la position, la hauteur ou la largeur */
	virtual void SetXY (int _x, int _y);                  /**< Set \a x and \a y positions */
	void SetHeight (uint _h);                             /**< Set \a height */
	void SetWidth (uint _w);                              /**< Set \a width */

	/* Visibilité */
	bool Visible() const { return visible; }              /**< Is this object visible ? */
	void Show() { visible = true; }                       /**< Set visible to true */
	void Hide() { visible = false; }                      /**< Set visible to false */
	void SetVisible(bool b = true) { visible = b; }

	bool Enabled() const { return enabled; }              /**< Is this object enabled ? */
	virtual void SetEnabled(bool _en = true)              /**< Set or unset this objet as enabled */
		{ enabled = _en; }

	/* Le composant a le focus ? */
	bool Focused() const { return focus; }
	virtual void SetFocus();
	virtual void DelFocus();

	virtual bool Test (int souris_x, int souris_y) const;
	virtual bool Clic (int mouse_x, int mouse_y) { return Test(mouse_x, mouse_y); }
	virtual void PressKey(SDL_keysym) { return; }

	void SetOnClick(TOnClickFunction c, void* param) { on_click_func = c; on_click_param = param; }
	TOnClickFunction OnClick() const { return on_click_func; }
	void* OnClickParam() const { return on_click_param; }

	void SetOnClickPos(TOnClickPosFunction c) { on_click_pos_func = c;  }
	TOnClickPosFunction OnClickPos() const { return on_click_pos_func; }

	void SetOnMouseOn(TOnMouseOnFunction m, void* param) { on_mouse_on_func = m; on_mouse_on_param = param; }
	TOnMouseOnFunction OnMouseOn() const { return on_mouse_on_func; }
	void* OnMouseOnParam() const { return on_mouse_on_param; }

	void SetForceFocus(bool b = true) { force_focus = b; }
	bool ForceFocus() const { return force_focus; }

	void SetHint(const char* h) { hint = h; }
	const char* Hint() { return hint.empty() ? "" : hint.c_str(); }
	bool DynamicHint() const { return dynamic_hint; }

	int Tag;

/* Variables privées */
protected:
	int x, y;
	uint h, w;
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
	bool dynamic_hint;
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
	void AddLine(TComponent *);

	/** Remove a component from list
	 * \warning This will use \a delete on TComponent !!
	 */
	bool RemoveLine(TComponent *c, bool use_delete = false);

	/** Draw all components in list */
	void Draw(int souris_x, int souris_y);

	/** Initialization not requiered... But it is a virtual function so i have to implemente this  */
	void Init() {}

	virtual bool Clic (int mouse_x, int mouse_y);

	virtual void DelFocus();

/* Attributs */
public:

	void SetXY (int _x, int _y); /** Reimplementation to affect all components in the list */

	/** Get list of components as a vector */
	ComponentVector GetList() const { return list; }

/* Variables privées */
private:

	/** Set \a x for all components in list, and reset height of TList */
	void Rebuild();

	ComponentVector list;
};

#endif /* EC_COMPONENT_H */
