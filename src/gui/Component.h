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

typedef unsigned int   uint;

/********************************************************************************************
 *                               TComponent                                                 *
 ********************************************************************************************/
/** Base of components. */
class TComponent
{
/* Constructeur/Deconstructeur */
public:

	/** Default constructor, set x, y, h and w to 0 and \a visible and \a enabled to true. */
	TComponent() : x(0), y(0), h(0), w(0), visible(true), enabled(true)
	{}

	/** Constructor with position
	 * @param _x x position
	 * @param _y y position
	 */
	TComponent(int _x, int _y)
		: x(_x), y(_y), h(0), w(0), visible(true), enabled(true)
	{}

	/** Constructor with position and size
	 * @param _x x position
	 * @param _y y position
	 * @param _w width of component
	 * @param _h heigh of component
	 */
	TComponent(int _x, int _y, uint _w, uint _h)
		: x(_x), y(_y), h(_h), w(_w), visible(true), enabled(true)
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

	bool Enabled() const { return enabled; }              /**< Is this object enabled ? */
	void SetEnabled(bool _en = true) { enabled = _en; }   /**< Set or unset this objet as enabled */

/* Variables privées */
protected:
	int x, y;
	uint h, w;
	bool visible;
	bool enabled;
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
