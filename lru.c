#include <stdlib.h>
#include "lru.h"
#include "cache.h"

// Add a cache entry index to the head of the LRU (Least Recently Used) list
// The LRU list tracks cache usage order for eviction decisions
// Returns pointer to the new LRU node
LRU_List *lru_push(cache *cache, int index)
{
	LRU_List *list = cache->lru;
	// Allocate new node
	LRU_List *node = (LRU_List*)malloc(sizeof(LRU_List));
	node->index = index;
	
	if (cache->lru_size>0)
	{
		// Insert into existing circular doubly-linked list
		node->next = list;
		node->prev = list->prev;
		list->prev = node;
		node->prev->next = node;
	}
	else
	{
		// First node - create circular links to self
		node->next = node;
		node->prev = node;
	}
	
	cache->lru_size++;
	return node;
}

// Remove the least recently used entry from the LRU list
// Returns the cache entry index of the evicted item
// Securely wipes the removed node before freeing
int64_t lru_pop(cache *cache, LRU_List *list)
{
	// Get index of LRU item (tail of circular list)
	int index = (list->prev) ? (list->prev->index) : (index = list->index);
	
	if (cache->lru_size>0)
	{
		// Remove tail node from circular list
		LRU_List *temp = list->prev;
		if (list->next!=list)
		{
			// More than one node - update circular links
			list->prev = list->prev->prev;
		}
		
		// Securely overwrite node data before freeing
		arc4random_buf(temp, sizeof(struct LRU_List));
		free(temp);
	}
	else
	{
		// Last node in list
		arc4random_buf(list, sizeof(struct LRU_List));
		free(list);
	}
	
	cache->lru_size--;
	
	return index;
}

