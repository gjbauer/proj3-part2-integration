#include <stdlib.h>
#include "fl.h"

// Push a new index onto the free list
// Returns the new head of the list
FL_LL *fl_push(FL_LL *list, int index)
{
	// Allocate memory for new node
	FL_LL *node = (FL_LL*)malloc(sizeof(FL_LL));
	node->index = index;
	
	// Insert at head of list
	node->next = list;
	
	return node;
}

// Remove and return the head of the free list
// Securely wipes the removed node before freeing
FL_LL *fl_pop(FL_LL *list)
{
	// Save pointer to next node (new head)
	FL_LL *temp = list->next;
	
	// Securely overwrite node data before freeing
	arc4random_buf(list, sizeof(struct FL_LL));
	free(list);
	
	return temp;
}

