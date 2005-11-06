/*
	$Id$

	------------------------------------------------------------------------
	ClanLib, the platform independent game SDK.

	This library is distributed under the GNU LIBRARY GENERAL PUBLIC LICENSE
	version 2. See COPYING for details.

	For a total list of contributers see CREDITS.

	------------------------------------------------------------------------

	File purpose:
		Double-linked list class template.
*/

#ifndef header_link
#define header_link

#include <stdlib.h>

//! common_component="Templates"	

class CL_Link
{
public:
	CL_Link *prev;
	CL_Link *next;
	void *instance;
};

template <class TYPE>
class CL_List
//: List class.
// The list class is used to manage double-linked lists of any type.
// <Put in some more info on how linked list work here>
//!also: CL_Array - Array class
//!also: CL_Stack - Stack class
//!also: CL_Queue - Queue class
{
//private:
public:		// <-- this public: SHOULD be private:
	CL_Link *first;
	CL_Link *last;
	int num_items;

public:
	CL_List()
	{
		first = NULL;
		last = NULL;
		num_items = 0;
	}
	//: Constructor which initializes an empty list.

	CL_List(const CL_List<TYPE> &clone)
	{
		first = NULL;
		last = NULL;
		num_items = 0;

		copy(clone);
	}
	//: Copy Constructor.
	//!param: clone - List to clone.

	~CL_List()
	{
		clear();
	}

	void clear()
	{
		CL_Link *cur_instance = first;
		while (cur_instance != NULL)
		{
			CL_Link *delete_instance = cur_instance;
			cur_instance = cur_instance->next;
			
//			chunk_alloc.put(delete_instance);
			delete delete_instance;
		}
		
		first = NULL;
		last = NULL;
		num_items = 0;
	}
	//: Empties the list by removing all items.

	void copy(const CL_List<TYPE> &other)
	{
		clear();
		add(other);
	}
	//: Copies the specified list.
	// All contents in the list will be deleted before the 
	// contents of the other list is copied.
	//!param: other - List to copy.
	
	void add(const CL_List<TYPE> &other)
	{
		CL_Link *cur = other.first;
		while (cur != NULL)
		{
			add((TYPE *) cur->instance);
			cur = cur->next;
		}
	}
	//: Adds another list to end of the list.
	//!param: other - List to be added.
	
	CL_Link *add(TYPE *item)
	{
//		CL_Link *cur = chunk_alloc.get();
		CL_Link *cur = new CL_Link;
		
		cur->instance = item;
		cur->prev = last;
		cur->next = NULL;
		
		if (first == NULL) first = cur;
		if (last != NULL) last->next = cur;
		last = cur;
		
		num_items++;

		return cur;
	}
	//: Adds the new item to the end of the list.
	//!param: item - Item to be added.
	//!retval: Link-item which has been created and added.

	TYPE *get_item(int number)
	{
		if (number<0 || number>num_items) return NULL;

		CL_Link *cur_instance = first;
		for (int i=0; i<number; i++)
		{
			cur_instance=cur_instance->next;
		}

		return (TYPE *) cur_instance->instance;
	}
	//: Returns the item at the specified position.
	//!param: number - Position of item to return.
	//!retval: Item if found, NULL otherwise.

	TYPE* operator[](int number)
	{
		return get_item(number);
	}
	//: Returns the item at the specified position.
	//!param: number - Position of item to return.
	//!retval: Item if found, NULL otherwise.
	
	int get_num_items()
	{
		return num_items;
	}
	//: Returns number of items available.
	//!retval: Number of items available
	
	TYPE *get_first()
	{
		if (first==NULL) return NULL;
		return (TYPE *) first->instance;
	}
	//: Returns the first item.
	//!retval: First item if available, NULL otherwise.
	
	TYPE *get_last()
	{
		if (last==NULL) return NULL;
		return (TYPE *) last->instance;
	}
	//: Returns the last item.
	//!retval: Last item if available, NULL otherwise.

	bool del(TYPE *item)
	{
		return del(find_link(item));
	}
	//: Deletes the specified item.
	//!param: item - Item to be deleted.
	//!retval: true if item was deleted, false otherwise.
	
	bool del(CL_Link *item)
	{
		if (item == NULL) return false;
		
		if (item->prev != NULL) item->prev->next = item->next;
		else first = item->next;

		if (item->next != NULL) item->next->prev = item->prev;
		else last = item->prev;
		
//		chunk_alloc.put(item);
		delete item;

		num_items--;
		
		return true;
	}
	//: Removes the specified item.
	//!param: item - Item to be removed.
	//!retval: true if item was deleted, false otherwise.
	
//private:
	CL_Link *find_link(TYPE *item)
	{
		CL_Link *cur = first;
		while (cur != NULL) 
		{
			if ((cur->instance) == item) return cur;
			cur = cur->next;
		}

		return NULL;
	}
};

template <class TYPE>
class CL_Iterator
{
private:
	CL_List<TYPE> *list;
	CL_Link *current;

public:
	CL_Iterator(CL_List<TYPE> *_list)
	{
		list = _list;
		current = NULL;
	}
	
	CL_Iterator(CL_List<TYPE> &_list)
	{
		list = &_list;
		current = NULL;
	}

	TYPE *first()
	{
		current = list->first;

		if (current == NULL) return NULL;
		return (TYPE *) (TYPE *) current->instance;
	}
	
	TYPE *last() 
	{
		current = list->last;

		if (current == NULL) return NULL;
		return (TYPE *) current->instance;
	}
	
	TYPE *prev() 
	{
		if (current == NULL) return last();
		current = current->prev;

		if (current == NULL) return NULL;
		return (TYPE *) current->instance;
	}

	TYPE *next()
	{
		if (current == NULL) return first();
		current = current->next;

		if (current == NULL) return NULL;
		return (TYPE *) current->instance;
	}

	void remove()
	{
		if (current == NULL) return;

		CL_Link *del_link=current;
		current=current->prev;
		list->del(del_link);
	}

	TYPE* operator()()
	{
		if (current == NULL) return NULL;

		return (TYPE *) current->instance;
	}
};

#endif

