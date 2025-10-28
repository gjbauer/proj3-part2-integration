#include <stdlib.h>
#include "lru.h"
#include "cache.h"

// Add a cache entry index to the global dirty list (GDL)
// The GDL tracks all dirty blocks regardless of inode or block type
// Returns pointer to the new GDL node
GDL *gdl_push(cache *cache, int index)
{
	GDL *list = cache->gdl;
	// Allocate new node
	GDL *node = (GDL*)malloc(sizeof(GDL));
	node->index = index;
	
	if (cache->gdl_size>0)
	{
		// Insert into existing circular doubly-linked list
		node->next = list;
		node->prev = list->prev;
		list->prev = node;
		node->prev->next = node;
	}
	else
	{
		// First node - no circular links yet
		node->next = NULL;
		node->prev = NULL;
	}
	
	cache->gdl_size++;
	return node;
}

// Remove a specific node from the global dirty list (GDL)
// Securely wipes the removed node before freeing
void gdl_pop(cache *cache, GDL *list)
{
	int index = index = list->index;
	
	GDL *temp = list;
	// Update neighboring nodes to bypass this node
	if (list->prev) list->prev->next = list->next;
	if (list->next) list->next->prev = list->prev;
	
	// Securely overwrite node data before freeing
	arc4random_buf(temp, sizeof(struct GDL));
	free(temp);
	
	cache->gdl_size--;
}

