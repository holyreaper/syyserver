/*
 * bd_list.h
 *
 *  Created on: 2011-2-18
 *      Author: neil
 */

#ifndef BD_LIST_H_
#define BD_LIST_H_

#include "assert.h"
#include <cstddef>

struct bd_node
{
	bd_node* prev;
	bd_node* next;
};

struct bd_list
{
	bd_node* head;
	bd_node* tail;
	size_t count;
};


inline bool bd_find_node(bd_list* list, bd_node* node)
{
	for(bd_node* n=list->head; n!=0; n=n->next)
	{
		if(n == node)
			return true;
	}
	return false;
}

inline void bd_push_front(bd_list* list, bd_node* node)
{
	if(list->tail ==0)
	{
		list->tail = node;
	}

	if(list->head != 0)
	{
		node->next = list->head;
		list->head->prev = node;
	}
	list->head = node;
	++(list->count);
}

inline bd_node* bd_pop_back(bd_list* list)
{
	if(list->tail == 0)
		return 0;

	bd_node* node = list->tail;

	if(node->prev == 0)
	{
		list->tail = 0;
		list->head = 0;
	}
	else
	{
		node->prev->next =  0;;
		list->tail = node->prev;
	}

	node->prev = 0;
	--list->count;
	return node;
}

inline void bd_remove_node(bd_list* list, bd_node* node)
{
	assert(bd_find_node(list, node));

	if(node->next!=0 && node->prev!=0)
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	else if(node->next!=0 && node->prev==0)
	{
		list->head = node->next;
		node->next->prev = 0;
	}
	else if(node->next==0 && node->prev!=0)
	{
		list->tail = node->prev;
		node->prev->next = 0;
	}
	else
	{
		list->head = 0;
		list->tail = 0;
	}

	node->next = 0;
	node->prev = 0;

	--list->count;
}


#endif /* TAURUS_IMP_H_ */

