/* src/Menu.cpp - Make a menu (by ClanBomber)
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

#include <ClanLib/Core/System/system.h>
#include <ClanLib/Display/Input/input.h>
#include <ClanLib/Display/Input/inputbuffer.h>
#include <ClanLib/Display/Input/keyboard.h>
#include <ClanLib/Sound/soundbuffer.h>
#include <ClanLib/Display/Display/display.h>
#include <ClanLib/Display/Display/surface.h>
#include <ClanLib/Display/Font/font.h>

#include "Main.h"
#include "Menu.h"

#include "Timer.h"

MenuItem::MenuItem( const CL_String _text, int _id, int _parent, EC_MenuFunc _func, unsigned int _flags )
{
	text = _text;
	id = _id;
	parent = _parent;
	func = _func;
	flags = _flags;
}

bool MenuItem::has_children()
{
	return children.get_num_items() > 0;
}

void MenuItem::add_child( MenuItem* child )
{
	children.add( child );
}

int MenuItem::get_id() const
{
	return id;
}

int MenuItem::get_parent() const
{
	return parent;
}

CL_String MenuItem::get_text()
{
	return text;
}

void MenuItem::set_text(const CL_String _text)
{
	text = _text;
}


MenuItem_Value::MenuItem_Value( const CL_String _text, int _id, int _parent, int _min, int _max, int _value, EC_MenuFunc _func, unsigned int _flags ) : MenuItem( _text, _id, _parent, _func, _flags )
{
	value = _value;
	min = _min;
	max = _max;
	test_value();
}

int MenuItem_Value::get_value()
{
	return value;
}

int MenuItem_Value::get_min()
{
	return min;
}

int MenuItem_Value::get_max()
{
	return max;
}

void MenuItem_Value::set_min( int _min )
{
	min = _min;
	test_value();
}

void MenuItem_Value::set_max( int _max )
{
	max = _max;
	test_value();
}

void MenuItem_Value::set_value( int _value )
{
	value = _value;
	test_value();
}

void MenuItem_Value::inc_value()
{
	value++;
	test_value();
}

void MenuItem_Value::dec_value()
{
	value--;
	test_value();
}

void MenuItem_Value::test_value()
{
	if (value > max)
	{
	  	value = max;
	}
	if (value < min)
	{
		value = min;
	}
}

// *********************
MenuItem_String::MenuItem_String(const CL_String _text, int _id, int _parent, CL_String _string, EC_MenuFunc _func, unsigned int _flags) : MenuItem(_text, _id, _parent, _func, _flags)
{
	string = _string;
}

CL_String& MenuItem_String::get_string()
{
	return string;
}

void MenuItem_String::set_string( CL_String  _string )
{
	string = _string;
}

// *********************
MenuItem_StringList::MenuItem_StringList(const CL_String _text, int _id, int _parent, CL_Array<CL_String>
                                        _string_list, int _value, EC_MenuFunc _func, unsigned int _flags)
  : MenuItem(_text, _id, _parent, _func, _flags)
{
	set_strings( _string_list );
	value = _value;
	test_value();
}

CL_String MenuItem_StringList::get_string()
{
	return *(string_list[value]);
}

CL_Array<CL_String> MenuItem_StringList::get_strings()
{
	return string_list;
}

void MenuItem_StringList::set_value( int _value )
{
	value = _value;
	test_value();
}

int MenuItem_StringList::get_value()
{
	return value;
}

void MenuItem_StringList::set_strings( CL_Array<CL_String>  _string_list )
{
	min = 0;
	max = _string_list.get_num_items() - 1;
	for (int i=min; i<=max; i++)
		string_list.add( new CL_String( *(_string_list[i]) ) );
}

void MenuItem_StringList::inc_value()
{
	value++;
	test_value();
}

void MenuItem_StringList::dec_value()
{
	value--;
	test_value();
}

void MenuItem_StringList::test_value()
{
	if (value > max)
	{
	  	value = max;
	}
	if (value < min)
	{
		value = min;
	}
}

// ************************
Menu::Menu( const CL_String& name, EuroConqApp* _app )
{
	app = _app;
	current_run_id = -1;
	current_selection = 1;

	key_buffer = new CL_InputBuffer( CL_Input::keyboards[0] );

	items.add( new MenuItem(name, -1, -2, NULL, 0) );
}

Menu::~Menu()
{
	delete key_buffer;
}

void Menu::redraw( int yoffset )
{
	MenuItem* current = get_item_by_id(current_run_id);

	int width = Resources::Font_big()->get_text_width( current->get_text() );

	CL_Iterator<MenuItem> item_counter( current->children );
	while (item_counter.next() != NULL)
	{
		int w = Resources::Font_big()->get_text_width( item_counter()->get_text() );
		if (item_counter()->get_type() == MenuItem::MT_VALUE)
		{
			w += 50;
		}
		if (item_counter()->get_type()==MenuItem::MT_STRING)
		{
			w += Resources::Font_big()->get_text_width( ((MenuItem_String*)item_counter())->get_string() );
		}
		if (item_counter()->get_type()==MenuItem::MT_STRINGLIST)
		{
			w += Resources::Font_big()->get_text_width( ((MenuItem_StringList*)item_counter())->get_string() );
		}
		width = max( w, width );
	}
	width += 30;

	int height = current->children.get_num_items()*40 + 110;

	int left_border = 400 - width/2;
	int right_border = 400 + width/2   +10;//bug in print_right?

	int vert = yoffset + 300 - height / 2;

	Resources::Titlescreen()->put_screen( 0, 0 );
	CL_Display::fill_rect( left_border-30,vert-20, right_border+20,vert+20+height, 0,0,0,0.5f);

	Resources::Font_big()->print_center( 400, vert, current->get_text() );
	vert += 80;

	int act_draw = 0;
	while (item_counter.next() != NULL)
	{
		act_draw++;
		if((item_counter()->flags & M_RETOUR))
			vert += 10;
		if (act_draw==current_selection)
		{
			if((item_counter()->flags & M_READ_ONLY))
				CL_Display::fill_rect( left_border-30,vert-5, right_border+20,vert+35,
				                       0.1f,0.1f,0.2f,0.55f);
			else
				CL_Display::fill_rect( left_border-30,vert-5, right_border+20,vert+35,
				                       0.5f,0.1f,0.1f,0.55f);
		}
		if (item_counter()->get_type() == MenuItem::MT_VALUE)
		{
			if ((((MenuItem_Value*)item_counter())->get_min() == 0) && (((MenuItem_Value*)item_counter())->get_max() == 1))
			{
				Resources::Font_big()->print_right( right_border, vert, CL_String( ((MenuItem_Value*)item_counter())->get_value() ? "Oui" : "Non" ) );
			}
			else
			{
				Resources::Font_big()->print_right( right_border, vert, CL_String( ((MenuItem_Value*)item_counter())->get_value() ) );
			}
			Resources::Font_big()->print_left( left_border, vert, item_counter()->get_text() );
		} else
		if (item_counter()->get_type()==MenuItem::MT_STRING)
		{
			Resources::Font_big()->print_right( right_border, vert, ((MenuItem_String*)item_counter())->get_string() );
			Resources::Font_big()->print_left( left_border, vert, item_counter()->get_text() );
		} else
		if (item_counter()->get_type()==MenuItem::MT_STRINGLIST)
		{
			Resources::Font_big()->print_right( right_border, vert, ((MenuItem_StringList*)item_counter())->get_string() );
			Resources::Font_big()->print_left( left_border, vert, item_counter()->get_text() );
		} else
		{
			Resources::Font_big()->print_center( 400, vert, item_counter()->get_text() );
		}
		vert += 40;
	}
	Resources::Font_small()->print_right( right_border+0, vert+20, "v "APP_VERSION );
	CL_Display::flip_display(true);
}

int Menu::execute()
{
	bool must_redraw = true;

	key_buffer->clear();

	while (1)
	{
		if (must_redraw)
		{
			redraw();
			must_redraw = false;
		}

		MenuItem* current = get_item_by_id(current_run_id);

		while (key_buffer->keys_left() == 0)
		{
			CL_System::sleep( 10 );
			CL_System::keep_alive();
		}
		if (key_buffer->peek_key().state != CL_Key::Pressed)
		{
			key_buffer->get_key();
			continue;
		}
		MenuItem *selected = current->children.get_item(current_selection-1);
		switch (key_buffer->get_key().id)
		{
			case CL_KEY_DOWN:
				current_selection++;
				if (current_selection > current->children.get_num_items()) current_selection = 1;
				must_redraw = true;
			break;
			case CL_KEY_UP:
				current_selection--;
				if (current_selection < 1) current_selection = current->children.get_num_items();
				must_redraw = true;
			break;
			case CL_KEY_RIGHT:
				if (selected->get_type() == MenuItem::MT_VALUE && !(selected->flags & M_READ_ONLY))
				{
					((MenuItem_Value*)(selected))->inc_value();
					return selected->get_id();
				}
				if (selected->get_type() == MenuItem::MT_STRINGLIST && !(selected->flags & M_READ_ONLY))
				{
					((MenuItem_StringList*)(selected))->inc_value();
					return selected->get_id();
				}
			break;
			case CL_KEY_LEFT:
				if (selected->get_type() == MenuItem::MT_VALUE && !(selected->flags & M_READ_ONLY))
				{
					((MenuItem_Value*)(selected))->dec_value();
					return selected->get_id();
				}
				if (selected->get_type() == MenuItem::MT_STRINGLIST && !(selected->flags & M_READ_ONLY))
				{
					((MenuItem_StringList*)(selected))->dec_value();
					return selected->get_id();
				}
			break;
			case CL_KEY_ENTER:
				if((selected->flags & M_READ_ONLY)) break;
				if (selected->has_children())
				{
					if(!selected->func || selected->func(true))
					{
						current_run_id = selected->get_id();
						current_selection = 1;
						must_redraw = true;
						break;
					}
					else
						return -1;
				}
				else if(!(selected->flags & M_RETOUR))
				{
					if (selected->get_type() == MenuItem::MT_STRING)
					{
						enter_string( (MenuItem_String*)(selected) );
					}
					return selected->get_id();
				}
				/* WARN: Pas de break ici c'est *normal*/
			case CL_KEY_ESCAPE:
				if (current->get_id() != -1)
				{
					if(current->func)
						current->func(false);
					current_run_id = current->get_parent();
					current_selection = 1;
					must_redraw = true;
				}
			break;
		}
	}
	return -1;
}

void Menu::scroll_in()
{
	Timer timer;
	float i = -600;

	Resources::Font_big();
	Resources::Titlescreen();

	while (i<0)
	{
		redraw((int)i);
		i += 600 * timer.time_elapsed(true);
	}
}

void Menu::scroll_out()
{
	Timer timer;
	float i = 0;

	while (i<600)
	{
		redraw((int)i);
		i += 600 * timer.time_elapsed(true);
	}
}

void Menu::enter_string(MenuItem_String* item)
{
	while (CL_Keyboard::get_keycode(CL_KEY_ENTER))
	{
		CL_System::keep_alive();
	}

	key_buffer->clear();
	CL_String new_string = item->get_string();

	while (1)
	{
		Resources::Titlescreen()->put_screen(0, 0, 0 );
		CL_Display::fill_rect( 200,300, 600,400, 0,0,0,0.5f);

		Resources::Font_big()->print_left( 230, 330, item->get_text() );

		Resources::Font_big()->print_left( 380, 330, new_string );

		CL_Display::flip_display();
		CL_System::keep_alive();

		while (key_buffer->peek_key().state != CL_Key::NoKey)
		{
	                if (key_buffer->peek_key().state != CL_Key::Pressed)
        	        {
                	        key_buffer->get_key();
				continue;
	                }

			switch (key_buffer->get_key().id)
			{
				case CL_KEY_ENTER:
					item->set_string( new_string );
					return;
				break;
				case CL_KEY_BACKSPACE:
					new_string = new_string.mid( 0, new_string.get_length()-1 );
				break;
				case CL_KEY_SPACE:
					new_string += " ";
				break;
				case CL_KEY_A:
					new_string += "a";
				break;
				case CL_KEY_B:
					new_string += "b";
				break;
				case CL_KEY_C:
					new_string += "c";
				break;
				case CL_KEY_D:
					new_string += "d";
				break;
				case CL_KEY_E:
					new_string += "e";
				break;
				case CL_KEY_F:
					new_string += "f";
				break;
				case CL_KEY_G:
					new_string += "g";
				break;
				case CL_KEY_H:
					new_string += "h";
				break;
				case CL_KEY_I:
					new_string += "i";
				break;
				case CL_KEY_J:
					new_string += "j";
				break;
				case CL_KEY_K:
					new_string += "k";
				break;
				case CL_KEY_L:
					new_string += "l";
				break;
				case CL_KEY_M:
					new_string += "m";
				break;
				case CL_KEY_N:
					new_string += "n";
				break;
				case CL_KEY_O:
					new_string += "o";
				break;
				case CL_KEY_P:
					new_string += "p";
				break;
				case CL_KEY_Q:
					new_string += "q";
				break;
				case CL_KEY_R:
					new_string += "r";
				break;
				case CL_KEY_S:
					new_string += "s";
				break;
				case CL_KEY_T:
					new_string += "t";
				break;
				case CL_KEY_U:
					new_string += "u";
				break;
				case CL_KEY_V:
					new_string += "v";
				break;
				case CL_KEY_W:
					new_string += "w";
				break;
				case CL_KEY_X:
					new_string += "x";
				break;
				case CL_KEY_Y:
					new_string += "y";
				break;
				case CL_KEY_Z:
					new_string += "z";
					break;
				case CL_KEY_KP_0:
					new_string += "0";
					break;
				case CL_KEY_KP_1:
					new_string += "1";
					break;
				case CL_KEY_2:
					new_string += "2";
					break;
				case CL_KEY_3:
					new_string += "3";
					break;
				case CL_KEY_4:
					new_string += "4";
					break;
				case CL_KEY_5:
					new_string += "5";
					break;
				case CL_KEY_6:
					new_string += "6";
					break;
				case CL_KEY_7:
					new_string += "7";
					break;
				case CL_KEY_8:
					new_string += "8";
					break;
				case CL_KEY_9:
					new_string += "9";
					break;
				case CL_KEY_KP_DECIMAL:
					new_string += ".";
					break;
			}
			if (new_string.get_length() == 1 && !(item->flags & M_NOFMAJ))
			{
				new_string.to_upper();
			}
		}
	}
}

void Menu::add_item( const CL_String& text, int id, unsigned int flags, int parent )
{
	MenuItem* new_item = new MenuItem( text, id, parent, NULL, flags );

	get_item_by_id(parent)->add_child(new_item);
	items.add( new_item );
}

/* Cette fonction permet de passer une fonction en argument */
void Menu::add_item( const CL_String& text, int id, EC_MenuFunc func, unsigned int flags, int parent )
{
	MenuItem* new_item = new MenuItem( text, id, parent, func, flags );

	get_item_by_id(parent)->add_child(new_item);
	items.add( new_item );
}

void Menu::add_value( const CL_String& text, int id, unsigned int flags, int parent, int min, int max,
                      int value )
{
	MenuItem_Value* new_item = new MenuItem_Value( text, id, parent, min, max, value, NULL, flags );

	get_item_by_id(parent)->add_child(new_item);
	items.add( new_item );
}

void Menu::add_string( const CL_String& text, int id, unsigned int flags, int parent, CL_String string )
{
	MenuItem_String*  new_item = new MenuItem_String( text, id, parent, string, NULL, flags);

	get_item_by_id(parent)->add_child(new_item);
	items.add( new_item );
}

void Menu::add_stringlist( const CL_String& text, int id, unsigned int flags, int parent,
                           CL_Array<CL_String> string_list, int cur_string )
{
	MenuItem_StringList*  new_item = new MenuItem_StringList( text, id, parent, string_list, cur_string, NULL, flags);

	get_item_by_id(parent)->add_child(new_item);
	items.add( new_item );
}

MenuItem* Menu::get_item_by_id( int id )
{
	CL_Iterator<MenuItem> item_counter(items);
	while (item_counter.next() != NULL)
	{
		if (item_counter()->get_id() == id)
		{
			return item_counter();
		}
	}
	return NULL;
}
