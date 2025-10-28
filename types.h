#ifndef TYPES_H
#define TYPES_H

#include "pci.h"
#include "fl.h"
#include "lru.h"
#include "dl.h"
#include "gdl.h"

// ==================== DISK INTERFACE ====================

/**
 * Disk interface structure for managing filesystem storage
 * Provides memory-mapped access to disk image file
 */
typedef struct DiskInterface {
    int disk_file;                   // File handle for the disk image
    void* disk_base;                 // Memory-mapped base address of disk
    uint64_t total_blocks;           // Total blocks available on disk
    bool is_mounted;                 // Whether filesystem is mounted
} DiskInterface;

/**
 * Represents a single cache entry containing a disk block
 */
typedef struct cache_entry_t
{
	bool dirty_bit;              // True if block has been modified and needs writeback
	int pin_count;               // Reference count for preventing eviction
	uint64_t block_number;       // Disk block number this entry represents
	uint64_t inode_number;       // Inode that owns this block (for data blocks)
	void *page_data;             // Pointer to the actual cached block data
	struct LRU_List *lru_pos;    // Position in LRU list for eviction policy
	struct GDL *gdl_pos;         // Position in global dirty list
} cache_entry_t;

/**
 * Main cache structure managing all cached disk blocks
 */
typedef struct cache
{
	int cache_size;              // Total number of cache entries
	int lru_size;                // Current size of LRU list
	int gdl_size;                // Current size of global dirty list
	cache_entry_t *cache;        // Array of cache entries
	PCI_HM *pci;                 // Primary Cache Index: maps block_number -> cache_index
	LRU_List *lru;               // LRU list head for eviction policy
	FL_LL *free_list;            // Free list of available cache slots
	DL_HM *dirty_list;           // Dirty list: maps inode_number -> dirty blocks
	GDL *gdl;                    // Global dirty list for sync operations
} cache;

#endif
