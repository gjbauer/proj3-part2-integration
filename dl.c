#include <stdlib.h>
#include <stdio.h>
#include <bsd/stdlib.h>
#include "dl.h"

// Add a block number to a dirty list
// Returns the new head of the list
DL_LL *dl_push(DL_LL *list, uint64_t block_number)
{
	// Allocate new node
	DL_LL *node = (DL_LL*)malloc(sizeof(DL_LL));
	node->block_number = block_number;
	
	// Insert at head if list exists
	if (list) node->next = list;
	
	return node;
}

// Remove the head of a dirty list
// Securely wipes the removed node before freeing
DL_LL *dl_pop(DL_LL *list)
{
	// Save pointer to new head
	DL_LL *temp = list->next;
	
	// Securely overwrite node data before freeing
	arc4random_buf(list, sizeof(struct DL_LL));
	free(list);
	
	return temp;
}

// Look up an inode's dirty list in the dirty list hashmap
// Returns pointer to the inode's dirty list entry, or NULL if not found
DL_HM_LL *dl_lookup(DL_HM *hashmap, uint64_t inode_number)
{
	DL_HM_LL *current;
	// Hash inode number to find correct bucket
	current = hashmap->HashMap[inode_number % HASHMAP_SIZE];
	
	// Walk the chain looking for matching inode
	while (current)
	{
		if (current->inode_number==inode_number)
		{
			return current;
		} else current = current->next;
	}
	
	return NULL;
}

// Find a specific block number in a dirty list
// Returns pointer to the list node containing the block, or NULL if not found
DL_LL *dl_find_block(DL_LL *list, uint64_t block_number)
{
	DL_LL *curr = list;
	// Linear search through the list
	while (curr)
	{
		if (curr->block_number == block_number) return curr;
		curr=curr->next;
	}
	
	return NULL;
}

// Insert a dirty block into the dirty list for a specific inode
// Creates a new inode entry if it doesn't exist, or adds to existing list
void dl_insert(DL_HM *hashmap, uint64_t inode_number, uint64_t block_number)
{
	// Look for existing inode entry
	DL_HM_LL *node = dl_lookup(hashmap, inode_number);
	if (!node) {
		// Create new inode entry
		node = malloc(sizeof(DL_HM_LL));
		node->inode_number = inode_number;
		node->list=NULL;
		
		// Insert at head of hash bucket
		node->next = hashmap->HashMap[inode_number % HASHMAP_SIZE];
		hashmap->HashMap[inode_number % HASHMAP_SIZE] = node;
		
		// Add first block to this inode's dirty list
		node->list = dl_push(node->list, block_number);
	}
	else
	{
		// Check if block already exists in this inode's dirty list
		DL_LL *entry = dl_find_block(node->list, block_number);
		if (!entry) node->list = dl_push(node->list, block_number);
	}
}

// Remove an entire inode's dirty list from the hashmap
// Called when all dirty blocks for an inode have been written to disk
void dl_delete(DL_HM *hashmap, uint64_t inode_number)
{
	DL_HM_LL *curr = hashmap->HashMap[inode_number % HASHMAP_SIZE];
	DL_HM_LL *prev;
	
	// Find the inode entry to delete
	while (curr)
	{
		if (curr->inode_number==inode_number)
		{
			break;
		} else {
			prev = curr;
			curr = curr->next;
		}
	}
	
	printf("Removing inode %lu from dirty list!\n", inode_number);
	// Update chain to bypass deleted node
	if (prev) {
		prev->next = curr->next;
	} else {
		// Deleting head of chain
		hashmap->HashMap[inode_number % HASHMAP_SIZE] = curr->next;
	}
	// Securely overwrite node data before freeing
	arc4random_buf(curr, sizeof(struct DL_HM_LL));
	free(curr);
}

// Remove a specific block from an inode's dirty list
// If this was the last block, removes the entire inode entry
void dl_remove_block(DL_HM *hashmap, uint64_t inode_number, uint64_t block_number)
{
	// Find the inode's dirty list
	DL_HM_LL *list = dl_lookup(hashmap, inode_number);
	if (list)
	{
		DL_LL *curr = list->list;
		DL_LL *prev=NULL;
		
		// Find the specific block to remove
		while (curr)
		{
			if (curr->block_number == block_number) break;
			prev=curr;
			curr=curr->next;
		}
		
		// Update chain to bypass deleted block
		if (prev)
		{
			prev->next = curr->next;
		}
		free(curr);
		
		// If list is now empty, remove entire inode entry
		if (!list->list)  dl_delete(hashmap, inode_number);
	}
}
