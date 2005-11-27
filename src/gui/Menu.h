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

#include "Main.h"
#include <string>
#include <vector>

/* Flags */
#define M_READ_ONLY		0x001
#define M_NOFMAJ		0x002
#define M_RETOUR		0x004

typedef bool (*EC_MenuFunc) (bool in);

class CL_InputBuffer;

class Menu;

class MenuItem
{
friend class Menu;

public:
	MenuItem( const std::string _text, int _id, int _parent, EC_MenuFunc _func, unsigned int _flags );
	virtual ~MenuItem() {};

	void add_child( MenuItem* child );
	bool has_children();
	int get_id() const;
	int get_parent() const;
	std::string get_text();
	void set_text( const std::string _text );

	typedef enum
	{
		MT_NORMAL,
		MT_VALUE,
		MT_STRING,
		MT_STRINGLIST,
	} MENUTYPE;

	virtual MENUTYPE get_type() { return MT_NORMAL; };

protected:
	std::string				text;
	int						id;
	int						parent;
	std::vector<MenuItem*>	children;
	EC_MenuFunc				func;
	unsigned int			flags;
};

class MenuItem_Value : public MenuItem
{
friend class Menu;
public:
	MenuItem_Value( const std::string _text, int _id, int _parent, int _min, int _max, int _value,
	                EC_MenuFunc _func = NULL, unsigned int _flags = 0 );
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
	MenuItem_String( const std::string _text, int _id, int _parent, std::string _string,
	                 EC_MenuFunc _func = NULL, unsigned int _flags = 0);
	virtual ~MenuItem_String() {};

	std::string& get_string();
	void set_string(std::string _string);

	virtual MENUTYPE get_type() { return MT_STRING; };

protected:
	std::string string;
};

class MenuItem_StringList : public MenuItem
{
friend class Menu;
public:
	MenuItem_StringList( const std::string _text, int _id, int _parent, std::vector<std::string>
	                     _string_list, int _value, EC_MenuFunc _func = 0, unsigned int _flags = 0 );
	virtual ~MenuItem_StringList() {};

	std::string get_string();
	std::vector<std::string> get_strings();
	void set_strings(std::vector<std::string> _string_list);
	void inc_value();
	void dec_value();
	int get_value();
	void set_value(int _value);

	virtual MENUTYPE get_type() { return MT_STRINGLIST; };

protected:
	std::vector<std::string> string_list;
	void test_value();
	int value;
	int min;
	int max;
};

class Menu
{
public:
	Menu( const std::string& name, EuroConqApp* _app );
	~Menu() {}

	void add_item( const std::string& text, int id, unsigned int flags, int parent=-1);
	void add_item( const std::string& text, int id, EC_MenuFunc func, unsigned int flags, int parent=-1);
	void add_value(const std::string& text, int id, unsigned int flags, int parent, int min, int max,
	               int value);
	void add_string(const std::string& text, int id, unsigned int flags, int parent, std::string string);
	void add_stringlist( const std::string& text, int id, unsigned int flags, int parent,
	                     std::vector<std::string> string_list, int cur_string );
	int execute();
	void scroll_in();
	void scroll_out();
	void redraw(int yoffset=0);
	MenuItem* get_item_by_id( int id );
	void go_main_menu(); /* Afficher le menu principal */

	static std::string EnterString(std::string label, std::string last_string, bool first_cap);

protected:
	EuroConqApp* lapp;
	std::vector<MenuItem*> items;
	int current_run_id;
	unsigned int current_selection;
	void enter_string(MenuItem_String* _item);
};

#endif
