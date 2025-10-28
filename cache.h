#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "disk.h"
#include "types.h"

/* In this case, we push to the head of the list and pop from the tail.
 * In the other case we can push and pop from the head. */
LRU_List *lru_push(cache *cache, int index);
int64_t lru_pop(cache *cache, LRU_List *list);

/* In this case, we push to the head of the list, and pop from
 * wherever in the list the given node is placed.
 */
GDL *gdl_push(cache *cache, int index);
void gdl_pop(cache *cache, GDL *list);

/**
 * Retrieve a block from cache, loading from disk if necessary
 */
void*
get_block(DiskInterface* disk, cache *cache, uint64_t inum, uint64_t pnum);

/**
 * Write data to a cached block, marking it dirty
 */
void
write_block(DiskInterface* disk, cache *cache, void *buf, uint64_t inum, uint64_t pnum);

/**
 * Sync all dirty blocks for a specific inode to disk
 */
void cache_fsync(DiskInterface* disk, cache *cache, uint64_t inum);

/**
 * Sync all dirty blocks in the cache to disk
 */
void cache_sync(DiskInterface* disk, cache *cache);

/**
 * Allocate and initialize a new cache structure
 */
cache*
alloc_cache();

/**
 * Free all memory associated with a cache structure
 */
void
free_cache(cache *cache);

#endif
