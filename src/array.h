/*
	$Id$

	------------------------------------------------------------------------
	ClanLib, the platform independent game SDK.

	This library is distributed under the GNU LIBRARY GENERAL PUBLIC LICENSE
	version 2. See COPYING for details.

	For a total list of contributers see CREDITS.

	------------------------------------------------------------------------

	File purpose:
		Expandable array class template.
*/

//! common_component="Templates"	

#ifndef header_array
#define header_array

#include <stdlib.h>

template<class TYPE>
class CL_Array
//: Array class.
// The array class is used to manage arrays of any type.
// <Put in some more info on how arrays work here>
// It handles automatically resizing if the array grows over the allocated
// space. 
//!also: CL_List - List class
//!also: CL_Stack - Stack class
//!also: CL_Queue - Queue class
{
private:
	TYPE **items;
	int max_items;
	int highest_available;
	int lowest_available;
	
public:
	CL_Array()
	{
		highest_available = 0;
		lowest_available = 0;
		max_items = 50;
		items = new TYPE*[max_items];
		
		for (int i=0; i<max_items; i++) items[i] = NULL;
	}
	//: Constructor which initializes an empty array with 50 elements.

	CL_Array(int start_entries)
	{
		highest_available = 0;
		lowest_available = 0;
		max_items = start_entries;
		items = new TYPE*[max_items];
		
		for (int i=0; i<max_items; i++) items[i] = NULL;
	}
	//: Constructor which initializes an empty array with the specified number of elements.
	//!param: start_entries - Number of elements to initialize.

	CL_Array(const CL_Array &clone)
	{
		highest_available = clone.highest_available;
		lowest_available = clone.lowest_available;
		max_items = clone.max_items;

		items = new TYPE*[max_items];
		for (int i=0;i<max_items;i++) items[i] = clone.items[i];
	}
	//: Copy Constructor.
	//!param: clone - Array to clone.

	~CL_Array()
	{
		delete [] items;
	}
	
	void clear()
	{
		for (int i=0; i<max_items; i++) items[i] = NULL;
		lowest_available = 0;
		highest_available = 0;
	}
	//: Empties the array.
	
	int add(TYPE *item)
	{
		int retval;

		if (lowest_available == max_items) expand();
		items[lowest_available] = item;
		retval = lowest_available;

		find_lowest_available();

		return retval;
	}
	//: Adds a new item at the end of the array.
	//!param: item - Item to be added.
	//!retval: Position of item added.

	int add(TYPE *item, int position)
	{
		if (position >= max_items) expand(position+50);
		items[position] = item;
		
		if (position == lowest_available) find_lowest_available();

		return position;
	}
	//: Adds a new item at the specified position.
	//!param: item - Item to be added.
	//!param: position - Item to be added.
	//!retval: Position of item added.

	bool del(int position)
	{
		if (position < 0) return false;
		if (position >= max_items) return false;

		if (items[position] == NULL) return false;
		items[position] = NULL;
		
		if (position < lowest_available) lowest_available = position;

		while (highest_available > lowest_available)
		{
			if (items[highest_available] != NULL) break;
			highest_available--;
		}
		
		return true;
	}
	//: Removes the item at the specified position.
	// The items after the deleted item are NOT moved forward in the array,
	// the item is simple deleted.
	//!param: position - Position of item to be deleted.
	//!retval: true if item was deleted, false otherwise.

	bool del(TYPE *item)
	{
		return del(get_num(item));
	}
	//: Removes the specified item.
	// The items after the deleted item are NOT moved forward in the array,
	// the item is simple deleted.
	//!param: item - Item to be removed.
	//!retval: true if item was deleted, false otherwise.

	int get_num(TYPE *find_item)
	{
		for (int i=0; i<highest_available; i++)
		{
			if (items[i] == find_item) return i;
		}
		
		return -1;
	}
	//: Returns the position of the specified item.
	//!param: find_item - Item to return position for.
	//!retval: Position of item if it was found, otherwise -1.

	TYPE *get_item(int number)
	{
		if (number < 0) return NULL;
		if (number >= max_items) return NULL;

		return items[number];
	}
	//: Returns the item at the specified position.
	//!param: number - Position of item to return.
	//!retval: Item if it was found, otherwise NULL.

	TYPE *operator[] (int number)
	{
		return get_item(number);
	}
	//: Returns the item at the specified position.
	//!param: number - Position of item to return.
	//!retval: Item if it was found, otherwise NULL.
	
	int get_num_items()
	{
		return highest_available;
	}
	//: Returns number of items available.
	//!retval: Number of items available.

	int get_alloc_size()
	{
		return max_items;
	}
	//: Returns number of items allocated (not necessary used).
	//!retval: Number of items allocated.
	
private:
	void expand(int index_size = -1)
	{
		if (index_size == -1) index_size = max_items*2;

		TYPE **old_array = items;
		items = new TYPE*[index_size];
		
		for (int i=0; i<max_items; i++)
		{
			items[i] = old_array[i];
		}
		
		for (int j=max_items; j<index_size; j++)
		{
			items[j] = NULL;
		}
		
		max_items = index_size;
	}
	
	void find_lowest_available()
	{
		while (lowest_available < max_items)
		{
			if (items[lowest_available] == NULL) break;
			lowest_available++;
		}

		if (lowest_available > highest_available) highest_available = lowest_available;
	}
};

#endif

