/* src/Menu.h - Header of Menu.cpp (by ClanBomber)
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

#ifndef Menu_h
#define Menu_h

#include "array.h"

typedef bool (*EC_MenuFunc) (void);

class ClanBomberApplication;

class CL_InputBuffer;

class Menu;

class MenuItem
{
friend class Menu;

public:
	MenuItem( const CL_String _text, int _id, int _parent, EC_MenuFunc _func );
	virtual ~MenuItem() {};

	void add_child( MenuItem* child );
	bool has_children();
	int get_id() const;
	int get_parent() const;
	CL_String get_text();
	void set_text( const CL_String _text );

	typedef enum
	{
		MT_NORMAL,
		MT_VALUE,
		MT_STRING,
		MT_STRINGLIST,
	} MENUTYPE;

	virtual MENUTYPE get_type() { return MT_NORMAL; };

protected:
	CL_String			text;
	int					id;
	int					parent;
	CL_List<MenuItem>	children;
	EC_MenuFunc			func;
};

class MenuItem_Value : public MenuItem
{
friend class Menu;
public:
	MenuItem_Value( const CL_String _text, int _id, int _parent, int _min, int _max, int _value, EC_MenuFunc _func = NULL );
	virtual ~MenuItem_Value() {};

	int get_value();
	void set_value(int _value);
	int get_min();
	int get_max();
	void set_min( int _min );
	void set_max( int _max );
	void inc_value();
	void dec_value();

	virtual MENUTYPE get_type() { return MT_VALUE; };

protected:
	void test_value();
	int value;
	int min;
	int max;
};

class MenuItem_String : public MenuItem
{
friend class Menu;
public:
	MenuItem_String( const CL_String _text, int _id, int _parent, CL_String _string, EC_MenuFunc _func = NULL);
	virtual ~MenuItem_String() {};

	CL_String& get_string();
	void set_string(CL_String _string);

	virtual MENUTYPE get_type() { return MT_STRING; };

protected:
	CL_String string;
};

class MenuItem_StringList : public MenuItem
{
friend class Menu;
public:
	MenuItem_StringList( const CL_String _text, int _id, int _parent, CL_Array<CL_String> _string_list, int _value, EC_MenuFunc _func = 0 );
	virtual ~MenuItem_StringList() {};

	CL_String get_string();
	CL_Array<CL_String> get_strings();
	void set_strings(CL_Array<CL_String> _string_list);
	void inc_value();
	void dec_value();
	int get_value();
	void set_value(int _value);

	virtual MENUTYPE get_type() { return MT_STRINGLIST; };

protected:
	CL_Array<CL_String> string_list;
	void test_value();
	int value;
	int min;
	int max;
};

class Menu
{
public:
	Menu( const CL_String& name, EuroConqApp* _app );
	~Menu();

	void add_item( const CL_String& text, int id, int parent=-1 );
	void add_item( const CL_String& text, int id, EC_MenuFunc func, int parent=-1 );
	void add_value( const CL_String& text, int id, int parent, int min, int max, int value );
	void add_string( const CL_String& text, int id, int parent, CL_String string );
	void add_stringlist( const CL_String& text, int id, int parent, CL_Array<CL_String> string_list, int cur_string );
	int execute();
	void scroll_in();
	void scroll_out();
	void redraw(int yoffset=0);
	MenuItem* get_item_by_id( int id );
protected:
	EuroConqApp* app;
	CL_List<MenuItem> items;
	int current_run_id;
	int current_selection;
	void enter_string(MenuItem_String* _item);

	CL_InputBuffer* key_buffer;
};

#endif
