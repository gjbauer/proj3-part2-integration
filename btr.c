#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "btr.h"
#include "disk.h"
#include "cache.h"

/**
 * Create a new B-tree node on disk
 * Allocates a disk block and initializes the node structure
 */
BTreeNode* btree_node_create(DiskInterface* disk, cache *cache, bool is_leaf)
{
	// Allocate a new disk block for this node
	int page = alloc_page(disk, cache);
	
	// Get pointer to the allocated block
	BTreeNode *node = (BTreeNode*)get_block(disk, cache, 0, page);
	
	// Initialize node metadata
	node->block_number = page;
	node->is_leaf = is_leaf;
	node->num_keys = 0;
	node->parent = 0;
	node->left_sibling = 0;
	node->right_sibling = 0;
	
	// Initialize all keys and children to 0
	for(int i=0; i<MAX_KEYS; i++) node->keys[i]=0;
	for(int i=0; i<=MAX_KEYS; i++) node->children[i]=0;
	
	return node;
}

/**
 * Free a B-tree node and return its disk block to the free pool
 */
void btree_node_free(DiskInterface* disk, cache *cache, BTreeNode* node)
{
	free_page(disk, cache, node->block_number);
}

/**
 * Read a B-tree node from disk into memory
 * Copies node data from disk block to provided structure
 */
int btree_node_read(DiskInterface* disk, uint64_t block_num, BTreeNode* node)
{
	int rv;
	BTreeNode *disk_node = (BTreeNode*)get_block(disk, block_num);
	
	// Copy node data from disk to memory structure
	void *ptr = memcpy((char*)node, (char*)disk_node, sizeof(BTreeNode));
	
	rv = (ptr==NULL) ? -1 : 0;
	
	return rv;
}

/**
 * Write a B-tree node from memory to disk
 * Copies node data from memory structure to disk block
 */
int btree_node_write(DiskInterface* disk, BTreeNode* node)
{
	int rv;
	BTreeNode *mem_node = (BTreeNode*)get_block(disk, node->block_number);
	
	// Copy node data from memory to disk
	void *ptr = memcpy((char*)mem_node, (char*)node, sizeof(BTreeNode));
	
	rv = (ptr==NULL) ? -1 : 0;
	
	return rv;
}

/**
 * Search for a key in the B-tree
 * Recursively traverses the tree to find the specified key
 */
uint64_t btree_search(DiskInterface* disk, uint64_t node_block, uint64_t key)
{
	BTreeNode *node = (BTreeNode*)get_block(disk, node_block);
	
	if (node->is_leaf) {
		// Iteratively search all keys
		for (int i = 0; i <= node->num_keys; i++) {
			if (node->keys[i] == key) {
				printf("Found key!\n");
				return node->children[i];
			}
		}
		printf("Did not find key!\n");
		return -1;  // Key not found in any subtree
	} else {
		// Recursive case: search all children
		for (int i = 0; i <= node->num_keys; i++) {
			if (node->children[i] != 0) {
				uint64_t result = btree_search(disk, node->children[i], key);
				if (result != -1) {
					return result;  // Found in child subtree
				}
			}
		}
		printf("Did not find key!\n");
		return -1;  // Key not found in any subtree
	}
}

/**
 * Find the depth of a node in the B-tree
 * Follows the leftmost path down to a leaf to determine depth
 */
int btree_find_depth(DiskInterface* disk, uint64_t node_block)
{
	BTreeNode *node = (BTreeNode*)get_block(disk, node_block);
	
	int depth=0;
	while (true) {
		if (node->is_leaf) {
			return depth;  // Reached a leaf, return current depth
		}
	
		// Follow the leftmost child
		if (node->children[0] != 0) {
			node = (BTreeNode*)get_block(disk, node->children[0]);
			depth++;
		} else {
			printf("ERROR: Node is not leaf and child not found!!\n");
			break;
		}
	}
	
	return depth;
}

/**
 * Find the height of the B-tree from a given node
 * Height is the number of levels from this node to the deepest leaf
 */
int btree_find_height(DiskInterface* disk, uint64_t node_block)
{
	BTreeNode *node = (BTreeNode*)get_block(disk, node_block);
	
	// Special case: single node tree
	if (node->parent==0 && node->children[0]==0) return 0;
	
	int height=0;
	// Follow leftmost path to count levels
	while (!node->is_leaf)
	{
		node = (BTreeNode*)get_block(disk, node->children[0]);
		height++;
	}
	return height;
}

/**
 * Find the minimum key in a B-tree subtree
 * Recursively follows the leftmost path to find the smallest key
 */
int btree_find_minimum(DiskInterface* disk, uint64_t root_block)
{
	BTreeNode *root = (BTreeNode*)get_block(disk, root_block);
	
	// Follow leftmost child until we reach a leaf
	if (root->is_leaf) return root->children[0];
	else return btree_find_minimum(disk, root->children[0]);
}

/**
 * Find the maximum key in a B-tree subtree
 * Recursively follows the rightmost path to find the largest key
 */
int btree_find_maximum(DiskInterface* disk, uint64_t root_block)
{
	BTreeNode *root = (BTreeNode*)get_block(disk, root_block);
	
	// Find the rightmost non-null child and recurse
	for (int i = root->num_keys; i >= 0; i--) {
		if (!root->is_leaf) {
			if (root->children[i] != 0) {
				return btree_find_maximum(disk, root->children[i]);
			}
		}
		else if (root->children[i]!=0) return root->children[i];
	}
	
	return 0;  // Should not reach here in a valid tree
}

/**
 * Insert a node into a non-full internal node
 * Finds the correct position and shifts existing children as needed
 */
int btree_insert_nonfull(DiskInterface* disk, BTreeNode *root, uint64_t key, uint64_t value)
{
	// Find the correct position for the new child
	int child_pos = 0;
	for(int j = 0; j <= root->num_keys; j++) {
		if(root->children[j] != 0) {
			if(root->keys[j] < key) {
				child_pos = j + 1;
			} else {
				child_pos = j;
				break;
			}
		} else break;
	}
	
	// Shift existing children to make room
	for(int j = root->num_keys - 1; j >= child_pos; j--) {
		root->children[j + 1] = root->children[j];
	}
	// Shift existing keys to make room
	for(int j = root->num_keys - 1; j >= child_pos; j--) {
		root->keys[j + 1] = root->keys[j];
	}
	
	// Insert the new child
	root->children[child_pos] = value;
	root->keys[child_pos] = key;
	root->num_keys++;
	
	
	printf("Placing key %lu at child position %d\n", key, child_pos);
	printf("Inode number = %lu\n", value);
	
	return 0;
}

int btree_insertion_search(DiskInterface* disk, uint64_t root_block, uint64_t key)
{
	BTreeNode *root = (BTreeNode*)get_block(disk, root_block);
	
	if (root->children[0] == 0) {
		return root->block_number;
	}
	
	BTreeNode *node = root;
	int current_depth = 0;
	while (current_depth < btree_find_depth(disk, root_block)) {
		int child_index = 0;
		
		for (int i = 0; i < node->num_keys; i++) {
			if (node->children[i] != 0) {
				uint64_t child_max = btree_find_maximum(disk, node->children[i]);
				if (key <= child_max) {
					child_index = i;
					break;
				}
				child_index = i + 1;
			}
		}
		
		if (child_index > node->num_keys) {
			child_index = node->num_keys;
		}
		
		bool descended = false;
		if (node->children[child_index] != 0) {
			node = (BTreeNode*)get_block(disk, node->children[child_index]);
			current_depth++;
			descended = true;
		} else {
			for (int i = node->num_keys; i >= 0; i--) {
				if (node->children[i] != 0) {
					node = (BTreeNode*)get_block(disk, node->children[i]);
					current_depth++;
					descended = true;
					break;
				}
			}
		}
		
		if (!descended) {
			break;
		}
	}
	
	return node->block_number;
}

int btree_insert(DiskInterface* disk, uint64_t root_block, uint64_t key, uint64_t value)
{
	//BTreeNode *node = btree_node_create(disk, true);
	//node->key = key;
	
	int target_block = btree_insertion_search(disk, root_block, key);
	BTreeNode *target = (BTreeNode*)get_block(disk, target_block);
	
	if (target->num_keys == MAX_KEYS) {
		if (target->parent != 0) {
			BTreeNode *parent = (BTreeNode*)get_block(disk, target->parent);
			int i;
			for(i=0; i<MAX_KEYS && parent->keys[i] < btree_find_maximum(disk, target->block_number) && parent->keys[i]!=0; i++);
			btree_split_child(disk, parent, i, target);
			target_block = btree_insertion_search(disk, root_block, key);
			target = (BTreeNode*)get_block(disk, target_block);
		} else {
			btree_split_root(disk, target);
			target_block = btree_insertion_search(disk, root_block, key);
			target = (BTreeNode*)get_block(disk, target_block);
		}
		btree_insert_nonfull(disk, target, key, value);
	}
	else btree_insert_nonfull(disk, target, key, value);
	
	return 0;
}

/**
 * Borrow a child from the left sibling to rebalance the tree
 * Used during deletion when a node becomes too small
 */
KeyValuePair *btree_borrow_left(DiskInterface* disk, BTreeNode *node)
{
	KeyValuePair *pair = malloc(sizeof(KeyValuePair));
	if (node->left_sibling==0) return NULL;  // No left sibling
	else {
		BTreeNode *left_sibling = (BTreeNode*)get_block(disk, node->left_sibling);
		
		// Can't borrow if sibling has minimum keys
		if (left_sibling->num_keys==MIN_KEYS) return NULL;
		else {
			// Find the rightmost child to borrow
			for (int i = left_sibling->num_keys; i >= 0; i--) {
				if (left_sibling->children[i] != 0) {
					if (i < left_sibling->num_keys) left_sibling->keys[i]=0;
					pair->key = left_sibling->children[i];
					pair->value = left_sibling->children[i];  // Return borrowed child
					left_sibling->children[i] = 0;
					left_sibling->num_keys--;
					break;
				}
			}
		}
	}
	return pair;
}

/**
 * Borrow a child from the right sibling to rebalance the tree
 * Used during deletion when a node becomes too small
 */
KeyValuePair *btree_borrow_right(DiskInterface* disk, BTreeNode *node)
{
	KeyValuePair *pair = malloc(sizeof(KeyValuePair));
	if (node->right_sibling==0) return NULL;  // No right sibling
	else {
		BTreeNode *right_sibling = (BTreeNode*)get_block(disk, node->right_sibling);
		
		// Can't borrow if sibling has minimum keys
		if (right_sibling->num_keys==MIN_KEYS) return NULL;
		else {
			// Borrow the leftmost child
			pair->key = right_sibling->keys[0];
			pair->value = right_sibling->children[0];
			
			// Shift all keys and children left
			for(int i = 0; i < MAX_KEYS; i++) {
				right_sibling->keys[i] = right_sibling->keys[i+1];
			}
			for(int i = 0; i <= MAX_KEYS; i++) {
				right_sibling->children[i] = right_sibling->children[i+1];
			}
			right_sibling->keys[MAX_KEYS-1] = 0;
			right_sibling->children[MAX_KEYS] = 0;
			right_sibling->num_keys--;
		}
	}
	return pair;
}

int btree_delete(DiskInterface* disk, uint64_t root_block, uint64_t key)
{
	BTreeNode *root = (BTreeNode*)get_block(disk, root_block);
	int i;
	for(i=0; i<MAX_KEYS && root->keys[i] < key && root->keys[i]!=0; i++);
	BTreeNode *node = (BTreeNode*)get_block(disk, root->children[i]);
	BTreeNode *borrowed;
	int rv;
	KeyValuePair *pair=NULL;
	if (root->num_keys==MIN_KEYS && root->parent!=0)
	{
		pair = btree_borrow_left(disk, root);
		if (pair==NULL) {
			pair = btree_borrow_right(disk, root);
			if (pair==NULL) {
				BTreeNode *grandparent = (BTreeNode*)get_block(disk, root->parent);
				int j;
				for(j=0; j<MAX_KEYS && grandparent->keys[j] < key && grandparent->keys[j]!=0; j++);
				btree_merge_children(disk, grandparent, j);
			}
		}
	}
	for(i=0; i<MAX_KEYS && root->keys[i] < key && root->keys[i]!=0; i++);
	printf("Removing key %ld from block %ld\n", key, root_block);
	for(int j=i; j<root->num_keys; j++)
	{
		root->keys[j] = root->keys[j+1];
	}
	for(int j=i; j<=root->num_keys; j++)
	{
		root->children[j] = root->children[j+1];
	}
	root->keys[root->num_keys-1] = 0;
	root->children[root->num_keys] = 0;
	root->num_keys--;
	if (root->num_keys==0)
	{
		root->keys[0] = btree_find_maximum(disk, root_block);
		root->num_keys++;
	}
	
	if (pair!=NULL) {
		btree_insert_nonfull(disk, root, pair->key, pair->value);
		free(pair);
	}
	
	return 0;
}

void btree_split_root(DiskInterface* disk, BTreeNode* root)
{
	BTreeNode *child_a = btree_node_create(disk, root->is_leaf);
	BTreeNode *child_b = btree_node_create(disk, root->is_leaf);
	child_a->right_sibling = child_b->block_number;
	child_b->left_sibling = child_a->block_number;
	
	for (int i = 0; i < MIN_KEYS; i++) {
		child_a->keys[i] = root->keys[i];
		child_a->children[i] = root->children[i];
		if (root->children[i] != 0 && !root->is_leaf) {
			BTreeNode *child = (BTreeNode*)get_block(disk, root->children[i]);
			child->parent = child_a->block_number;
		}
		child_a->num_keys++;
	}
	
	for (int i = MIN_KEYS; i < root->num_keys; i++) {
		child_b->keys[i - MIN_KEYS] = root->keys[i];
		child_b->num_keys++;
	}
	for (int i = MIN_KEYS; i < root->num_keys; i++) {
		child_b->children[i - MIN_KEYS] = root->children[i];
		if (root->children[i] != 0 && !root->is_leaf) {
			BTreeNode *child = (BTreeNode*)get_block(disk, root->children[i]);
			child->parent = child_b->block_number;
		}
	}
	
	if (root->children[MAX_KEYS] != 0) {
		child_b->children[child_b->num_keys] = root->children[MAX_KEYS];
		BTreeNode *child = (BTreeNode*)get_block(disk, root->children[MAX_KEYS]);
		child->parent = child_b->block_number;
	}
	
	root->is_leaf = false;
	root->num_keys = 1;
	root->children[0] = child_a->block_number;
	root->children[1] = child_b->block_number;
	
	// Clear remaining slots
	for (int i = 1; i < MAX_KEYS; i++) {
		root->keys[i] = 0;
	}
	for (int i = 2; i <= MAX_KEYS; i++) {
		root->children[i] = 0;
	}
	
	child_a->parent = root->block_number;
	child_b->parent = root->block_number;
	root->keys[0] = btree_find_maximum(disk, child_a->block_number);
}

void btree_promote_root(DiskInterface* disk, BTreeNode* root)
{
	int page = root->block_number;
	
	BTreeNode *child = (BTreeNode*)get_block(disk, root->children[0]);
	
	memcpy(root, child, sizeof(BTreeNode));
	
	root->block_number = page;
	root->parent = 0;
	
	btree_node_free(disk, child);
	
	for (int i = 0; i <= root->num_keys; i++) {
		if (root->children[i] != 0) {
			child = (BTreeNode*)get_block(disk, root->children[i]);
			child->parent = root->block_number;
		}
	}
}

void btree_split_child(DiskInterface* disk, BTreeNode* node, int index, BTreeNode* child)
{
	BTreeNode *child_b = btree_node_create(disk, child->is_leaf);
	child_b->parent = node->block_number;
	child->right_sibling = child_b->block_number;
	child_b->left_sibling = child->block_number;
	
	for (int i = MIN_KEYS; i < child->num_keys; i++) {
		child_b->keys[i - MIN_KEYS] = child->keys[i];
		child->keys[i] = 0;
		child_b->num_keys++;
	}
	
	for (int i = MIN_KEYS; i <= child->num_keys; i++) {
		child_b->children[i - MIN_KEYS] = child->children[i];
		if (child_b->children[i - MIN_KEYS] != 0 && !child_b->is_leaf) {
			BTreeNode *grandchild = (BTreeNode*)get_block(disk, child_b->children[i - MIN_KEYS]);
			grandchild->parent = child_b->block_number;
		}
		child->children[i] = 0;
	}
	
	child->num_keys = MIN_KEYS;
	
	if (node->num_keys < MAX_KEYS) {
		for (int i = node->num_key - 1s; i > index; i--) {
			node->children[i + 1] = node->children[i];
		}
		// Shift keys to make room for new key
		for (int i = node->num_keys - 1; i > index; i--) {
			node->keys[i + 1] = node->keys[i];
		}
		
		node->children[index + 1] = child_b->block_number;
		node->keys[index + 1] = btree_find_maximum(disk, child_b->block_number);
		child_b->parent = node->block_number;
		node->num_keys++;
		for (int i = 0; i < node->num_keys; i++) {
			if (node->children[i] != 0) {
				node->keys[i] = btree_find_maximum(disk, node->children[i]);
			}
		}
	} else {
		if (node->parent != 0) {
			BTreeNode *grandparent = (BTreeNode*)get_block(disk, node->parent);
			int parent_index;
			for (parent_index = 0; parent_index <= grandparent->num_keys; parent_index++) {
				if (grandparent->children[parent_index] == node->block_number) break;
			}
			btree_split_child(disk, grandparent, parent_index, node);
		} else {
			btree_split_root(disk, node);
		}
		
		BTreeNode *current_parent = (BTreeNode*)get_block(disk, child->parent);
		int new_index;
		for (new_index = 0; new_index <= current_parent->num_keys; new_index++) {
			if (current_parent->children[new_index] == child->block_number) break;
		}
		
		if (current_parent->num_keys < MAX_KEYS) {
			for (int i = current_parent->num_keys; i > new_index; i--) {
				current_parent->children[i + 1] = current_parent->children[i];
			}
			
			current_parent->children[new_index + 1] = child_b->block_number;
			current_parent->keys[new_index] = btree_find_maximum(disk, current_parent->children[new_index]);
			current_parent->num_keys++;
		}
	}
}

void btree_merge_children(DiskInterface* disk, BTreeNode* parent, int index)
{
	if (index == MAX_KEYS) return btree_merge_children(disk, parent, index-1);
	else if (parent->children[index] == 0 || parent->children[index+1]==0) return;
	BTreeNode *child_a = (BTreeNode*)get_block(disk, parent->children[index]);
	BTreeNode *child_b = (BTreeNode*)get_block(disk, parent->children[index+1]);
	child_a->right_sibling = child_b->right_sibling;
	
	for (int i = MIN_KEYS + 1; i < MAX_KEYS; i++) {
		child_a->keys[i] = child_b->keys[i - MIN_KEYS - 1];
		child_a->num_keys++;
	}
	for (int i = MIN_KEYS + 1; i <= MAX_KEYS; i++) {
		child_a->keys[i] = child_b->children[i - MIN_KEYS - 1];
	}
	
	for(int i=index+1; i<MAX_KEYS-1; i++)
	{
		parent->keys[i] = parent->keys[i+1];
	}
	parent->keys[MAX_KEYS - 1] = 0;
	
	for(int i=index+1; i<MAX_KEYS; i++)
	{
		parent->children[i] = parent->children[i+1];
	}
	
	btree_node_free(disk, child_b);
	
	parent->num_keys--;
	
	if (parent->num_keys < MIN_KEYS) {
		if (parent->parent == 0) {
			if (parent->children[1]==0) {
				printf("Promoting root!\n");
				btree_promote_root(disk, parent);
			}
		} else {
			KeyValuePair *pair = btree_borrow_left(disk, parent);
			if (pair==NULL) {
				pair = btree_borrow_right(disk, parent);
				if (pair==NULL) {
					BTreeNode *grandparent = (BTreeNode*)get_block(disk, parent->parent);
					int j;
					for(j=0; j<MAX_KEYS && grandparent->keys[j] < btree_find_maximum(disk, parent->block_number) && grandparent->keys[j]!=0; j++);
					btree_merge_children(disk, grandparent, j);
				}
			}
		}
	}
}

/**
 * Print B-tree structure for debugging
 * Recursively traverses and displays the tree with indentation showing levels
 */
void btree_print(DiskInterface* disk, uint64_t root_block, int level)
{
	BTreeNode *node = (BTreeNode*)get_block(disk, root_block);
	printf("%*sBlock %lu: ", level*2, "", root_block);  // Indent based on level
	
	// Print internal node information
	printf("INTERNAL keys=[");
	for(int i = 0; i < node->num_keys; i++) {
		printf("%lu", node->keys[i]);
		if (i < node->num_keys-1) printf(",");
	}
	printf("] children=[");
	for(int i = 0; i <= node->num_keys; i++) {
		printf("%lu", node->children[i]);
		if (i < node->num_keys) printf(",");
	}
	printf("] ");
	printf("Parent=%lu\n", node->parent);
	
	// Recursively print all children with increased indentation
	for(int i = 0; i <= node->num_keys; i++) {
		if (node->children[i] != 0 && !node->is_leaf) {
			btree_print(disk, node->children[i], level+1);
		}
	}
}

