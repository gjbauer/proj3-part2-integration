#ifndef BTR_H
#define BTR_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "disk.h"

/**
 * B-tree implementation for filesystem indexing
 * Supports insertion, deletion, and search operations
 * Uses disk-based storage with memory-mapped access
 */

/**
 * B-tree node structure stored on disk
 * Each node occupies one disk block
 */
typedef struct BTreeNode {
    uint64_t block_number;		// Physical block number on disk where this node is stored
    bool is_leaf;			// Whether this is a leaf node (contains actual data)
    uint16_t num_keys;			// Current number of keys stored in this node
    uint64_t keys[MAX_KEYS];		// Array of keys (could be inode numbers or other identifiers)
    uint64_t children[MAX_KEYS + 1];	// Array of child block numbers (internal nodes only)
    uint64_t parent;			// Parent node block number (0 if root)
    uint64_t left_sibling;		// Block number of left sibling (for efficient traversal)
    uint64_t right_sibling;		// Block number of right sibling (for efficient traversal)
} BTreeNode;

typedef struct KeyValuePair
{
	int key;
	int value;
} KeyValuePair;

// ==================== B-TREE OPERATIONS ====================

// ==================== NODE MANAGEMENT ====================

/**
 * Create a new B-tree node on disk
 * @param disk Pointer to DiskInterface
 * @param is_leaf Whether the new node should be a leaf
 * @return Pointer to newly created node
 */
BTreeNode* btree_node_create(DiskInterface* disk, cache *cache, bool is_leaf);

/**
 * Free a B-tree node and return its disk block to free pool
 * @param disk Pointer to DiskInterface
 * @param node Pointer to node to free
 */
void btree_node_free(DiskInterface* disk, cache *cache, BTreeNode* node);

/**
 * Read a B-tree node from disk into memory
 * @param disk Pointer to DiskInterface
 * @param block_num Block number to read from
 * @param node Pointer to node structure to populate
 * @return 0 on success, -1 on failure
 */
int btree_node_read(DiskInterface* disk, uint64_t block_num, BTreeNode* node);

/**
 * Write a B-tree node from memory to disk
 * @param disk Pointer to DiskInterface
 * @param node Pointer to node to write
 * @return 0 on success, -1 on failure
 */
int btree_node_write(DiskInterface* disk, BTreeNode* node);

// ==================== CORE B-TREE OPERATIONS ====================

/**
 * Search for a key in the B-tree
 * @param disk Pointer to DiskInterface
 * @param root_block Block number of root node
 * @param key Key to search for
 * @return Block number containing the key, or -1 if not found
 */
uint64_t btree_search(DiskInterface* disk, uint64_t root_block, uint64_t key);

/**
 * Insert a key into the B-tree
 * @param disk Pointer to DiskInterface
 * @param root_block Block number of root node
 * @param key Key to insert
 * @return 0 on success, -1 on failure
 */
int btree_insert(DiskInterface* disk, uint64_t root_block, uint64_t key, uint64_t value);

/**
 * Delete a key from the B-tree
 * @param disk Pointer to DiskInterface
 * @param root_block Block number of root node
 * @param key Key to delete
 * @return Block number of deleted node, or -1 if key not found
 */
int btree_delete(DiskInterface* disk, uint64_t root_block, uint64_t key);

// ==================== INTERNAL OPERATIONS ====================

/**
 * Split the root node when it becomes full
 * Creates two new child nodes and updates root
 * @param disk Pointer to DiskInterface
 * @param root Pointer to root node to split
 */
void btree_split_root(DiskInterface* disk, BTreeNode* root);

/**
 * Split a full child node
 * @param disk Pointer to DiskInterface
 * @param node Pointer to parent node
 * @param index Index of child to split
 * @param child Pointer to child node to split
 */
void btree_split_child(DiskInterface* disk, BTreeNode* node, int index, BTreeNode* child);

/**
 * Merge two adjacent child nodes when they become too small
 * @param disk Pointer to DiskInterface
 * @param parent Pointer to parent node
 * @param index Index of first child to merge
 */
void btree_merge_children(DiskInterface* disk, BTreeNode* parent, int index);

// ==================== DEBUGGING ====================

/**
 * Print B-tree structure for debugging
 * @param disk Pointer to DiskInterface
 * @param root_block Block number of root node
 * @param level Current indentation level for pretty printing
 */
void btree_print(DiskInterface* disk, uint64_t root_block, int level);

#endif

