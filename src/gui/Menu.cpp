/* src/Menu.cpp - Make a menu (by ClanBomber)
 *
 * Copyright (C) 2005-2006 Romain Bignon  <Progs@headfucking.net>
 *                         Laurent Defert <lodesi2@yahoo.fr>
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

#include <algorithm>

#include "Main.h"
#include "Menu.h"
#include "Resources.h"
#include "lib/Outils.h"
#include "tools/Font.h"

#include "Timer.h"

MenuItem::MenuItem(const std::string _text, int _id, int _parent, EC_MenuFunc _func, unsigned int _flags)
{
	text = _text;
	id = _id;
	parent = _parent;
	func = _func;
	flags = _flags;
}

bool MenuItem::has_children()
{
	return children.size() > 0;
}

void MenuItem::add_child( MenuItem* child )
{
	children.push_back(child);
}

int MenuItem::get_id() const
{
	return id;
}

int MenuItem::get_parent() const
{
	return parent;
}

std::string MenuItem::get_text()
{
	return text;
}

void MenuItem::set_text(const std::string _text)
{
	text = _text;
}


MenuItem_Value::MenuItem_Value(const std::string _text, int _id, int _parent, int _min, int _max, int _value, EC_MenuFunc _func, unsigned int _flags ) : MenuItem( _text, _id, _parent, _func, _flags )
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
MenuItem_String::MenuItem_String(const std::string _text, int _id, int _parent, std::string _string, EC_MenuFunc _func, unsigned int _flags) : MenuItem(_text, _id, _parent, _func, _flags)
{
	string = _string;
}

std::string& MenuItem_String::get_string()
{
	return string;
}

void MenuItem_String::set_string( std::string  _string )
{
	string = _string;
}

// *********************
MenuItem_StringList::MenuItem_StringList(const std::string _text, int _id, int _parent,
	        std::vector<std::string> _string_list, int _value, EC_MenuFunc _func, unsigned int _flags)
  : MenuItem(_text, _id, _parent, _func, _flags)
{
	set_strings( _string_list );
	value = _value;
	test_value();
}

std::string MenuItem_StringList::get_string()
{
	return string_list[value];
}

std::vector<std::string> MenuItem_StringList::get_strings()
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

void MenuItem_StringList::set_strings( std::vector<std::string>  _string_list )
{
	min = 0;
	max = _string_list.size() - 1;
	for (int i=min; i<=max; i++)
		string_list.push_back( std::string( _string_list[i] ) );
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
Menu::Menu( const std::string& name, EuroConqApp* _app )
{
	lapp = _app;
	current_run_id = -1;
	current_selection = 1;

	items.push_back( new MenuItem(name, -1, -2, NULL, 0) );
}

void Menu::go_main_menu()
{
	current_run_id = -1;
	redraw();
}

void Menu::redraw( int yoffset )
{
	MenuItem* current = get_item_by_id(current_run_id);

  const int space_between_items = 15;
	int item_height = big_font.GetHeight("A") + space_between_items;

  int width=0;
	std::vector<MenuItem*>::iterator item_counter = current->children.begin();
	for(;item_counter != current->children.end(); ++item_counter)
	{
		int w = big_font.GetWidth( (*item_counter)->get_text() );
		if ((*item_counter)->get_type() == MenuItem::MT_VALUE)
		{
			w += 250;
		}
		if ((*item_counter)->get_type()==MenuItem::MT_STRING)
		{
			w += big_font.GetWidth( ((MenuItem_String*)(*item_counter))->get_string() );
		}
		if ((*item_counter)->get_type()==MenuItem::MT_STRINGLIST)
		{
			w += big_font.GetWidth( ((MenuItem_StringList*)(*item_counter))->get_string() );
		}
		width = std::max( w, width );
	}
#ifdef DEBUG_MENU
	printf("width=%d\n", width);
#endif

	int left_border = 400 - width/2;
	int right_border = 400 + width/2;

	//Resources::Titlescreen()->Draw();
	SDL_BlitSurface(Resources::Titlescreen()->Img,NULL,lapp->sdlwindow,NULL);
	//CL_Display::fill_rect( left_border-30,yoffset-20, right_border+20,yoffset+20+height, 0,0,0,0.0f);
  yoffset += 160;
	big_font.WriteCenterTop(400,yoffset, current->get_text(), black_color);
	yoffset += item_height * 2;

	unsigned int act_draw = 0;
	for(item_counter = current->children.begin();item_counter != current->children.end(); ++item_counter)
	{
		act_draw++;
		if(((*item_counter)->flags & M_RETOUR))
			yoffset += 10;
		if (act_draw==current_selection)
		{
			SDL_Rect r_back = {left_border-15,yoffset-space_between_items/2, width+30, item_height};
			if(((*item_counter)->flags & M_READ_ONLY))
				SDL_FillRect(lapp->sdlwindow, &r_back, SDL_MapRGBA(lapp->sdlwindow->format, 15,15,15,55));
			else
				SDL_FillRect(lapp->sdlwindow, &r_back, SDL_MapRGBA(lapp->sdlwindow->format, 10,60,10,55));

		}
		if ((*item_counter)->get_type() == MenuItem::MT_VALUE)
		{
			if ((((MenuItem_Value*)(*item_counter))->get_min() == 0) &&
			    (((MenuItem_Value*)(*item_counter))->get_max() == 1))
			{
				big_font.WriteRight( right_border, yoffset,
			                std::string(((MenuItem_Value*)(*item_counter))->get_value() ? "Oui" : "Non"),
			                black_color);
			}
			else
			{
				big_font.WriteRight(right_border, yoffset,
				                 TypToStr(((MenuItem_Value*)(*item_counter))->get_value()), black_color);
			}
			big_font.WriteLeft( left_border, yoffset, (*item_counter)->get_text(), black_color );
		}
		else if ((*item_counter)->get_type()==MenuItem::MT_STRING)
		{
			if(!((MenuItem_String*)(*item_counter))->get_string().empty())
				big_font.WriteRight(right_border, yoffset,
				                    ((MenuItem_String*)(*item_counter))->get_string(), black_color);
			big_font.WriteLeft(left_border, yoffset, (*item_counter)->get_text(), black_color);
		}
		else if ((*item_counter)->get_type()==MenuItem::MT_STRINGLIST)
		{
			big_font.WriteRight(right_border, yoffset,
			                    ((MenuItem_StringList*)(*item_counter))->get_string(), black_color);
			big_font.WriteLeft( left_border, yoffset, (*item_counter)->get_text(), black_color);
		}
		else
		{
			big_font.WriteCenterTop( 400, yoffset, (*item_counter)->get_text(), black_color );
		}
		yoffset += item_height;
	}
  int version_h = normal_font.GetHeight("v "APP_VERSION);
	normal_font.WriteRight( 800-25, 600-version_h/2-25, "v "APP_VERSION, white_color );
	SDL_Flip(lapp->sdlwindow);
}

int Menu::execute()
{
	bool must_redraw = true;
	SDL_Event event;

	while (1)
	{
		if (must_redraw)
		{
			redraw();
			must_redraw = false;
		}

		MenuItem* current = get_item_by_id(current_run_id);

		MenuItem *selected;
		while( SDL_PollEvent( &event) )
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					selected = current->children[current_selection-1];
					switch (event.key.keysym.sym)
					{
						case SDLK_DOWN:
							current_selection++;
							if (current_selection > current->children.size()) current_selection = 1;
							must_redraw = true;
						break;
						case SDLK_UP:
							current_selection--;
							if (current_selection < 1) current_selection = current->children.size();
							must_redraw = true;
						break;
						case SDLK_RIGHT:
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
						case SDLK_LEFT:
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
						case SDLK_RETURN:
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
						case SDLK_ESCAPE:
							if (current->get_id() != -1)
							{
								if(current->func)
									current->func(false);
								current_run_id = current->get_parent();
								current_selection = 1;
								//must_redraw = true;
								return -1;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}
	}
	return -1;
}

void Menu::scroll_in()
{
	Timer timer;
	float i = -600;

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

std::string Menu::EnterString(std::string label, std::string last_string, bool first_cap)
{
	std::string new_string = last_string;

	SDL_Event event;

	while (1)
	{
		SDL_BlitSurface(Resources::Menuscreen()->Img,NULL,app.sdlwindow,NULL);

		big_font.WriteLeft( 230, 330, label, black_color );
		big_font.WriteLeft( 380, 330, new_string+"_", black_color );

		SDL_Flip(app.sdlwindow);

		while (SDL_PollEvent( &event))
		{
			if (event.type == SDL_KEYUP)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					return last_string;
				else if(event.key.keysym.sym == SDLK_RETURN)
					return new_string;
				continue;
			}

			switch (event.key.keysym.sym)
			{
				case SDLK_BACKSPACE:
					new_string = std::string(new_string, 0, new_string.size()-1 );
					break;
				case SDLK_SPACE:
					new_string += " ";
					break;
					break;
				case SDLK_KP_PERIOD:
					new_string += ".";
					break;
				default:
          //Alpha keys
          if(event.key.keysym.sym >= SDLK_a
          && event.key.keysym.sym <= SDLK_z)
          {
            char c = 'a' + event.key.keysym.sym - SDLK_a;
            new_string += c;
          }
          //Numeric keys
          if(event.key.keysym.sym >= SDLK_0
          && event.key.keysym.sym <= SDLK_9)
          {
            char c = '0' + event.key.keysym.sym - SDLK_0;
            new_string += c;
          }
					break;
			}
			if (new_string.size() == 1 && first_cap)
				std::transform( new_string.begin(), new_string.end(), new_string.begin(), static_cast<int (*)(int)>(toupper) );
		}
	}
}

void Menu::enter_string(MenuItem_String* item)
{
	item->set_string(Menu::EnterString(item->get_text(), item->get_string(), !(item->flags & M_NOFMAJ)));
	return;
}

void Menu::add_item( const std::string& text, int id, unsigned int flags, int parent )
{
	MenuItem* new_item = new MenuItem( text, id, parent, NULL, flags );

	get_item_by_id(parent)->add_child(new_item);
	items.push_back( new_item );
}

/* Cette fonction permet de passer une fonction en argument */
void Menu::add_item( const std::string& text, int id, EC_MenuFunc func, unsigned int flags, int parent )
{
	MenuItem* new_item = new MenuItem( text, id, parent, func, flags );

	get_item_by_id(parent)->add_child(new_item);
	items.push_back( new_item );
}

void Menu::add_value( const std::string& text, int id, unsigned int flags, int parent, int min, int max,
                      int value )
{
	MenuItem_Value* new_item = new MenuItem_Value( text, id, parent, min, max, value, NULL, flags );

	get_item_by_id(parent)->add_child(new_item);
	items.push_back( new_item );
}

void Menu::add_string( const std::string& text, int id, unsigned int flags, int parent,
                       std::string string )
{
	MenuItem_String*  new_item = new MenuItem_String( text, id, parent, string, NULL, flags);

	get_item_by_id(parent)->add_child(new_item);
	items.push_back( new_item );
}

void Menu::add_stringlist( const std::string& text, int id, unsigned int flags, int parent,
                           std::vector<std::string> string_list, int cur_string )
{
	MenuItem_StringList*  new_item = new MenuItem_StringList( text, id, parent, string_list, cur_string, NULL, flags);

	get_item_by_id(parent)->add_child(new_item);
	items.push_back( new_item );
}

MenuItem* Menu::get_item_by_id( int id )
{
	std::vector<MenuItem*>::iterator item_counter = items.begin();
	for(;item_counter != items.end(); ++item_counter)
	{
		if ((*item_counter)->get_id() == id)
		{
			return *item_counter;
		}
	}
	return NULL;
}
