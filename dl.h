#ifndef DL_H
#define DL_H
#include <stdint.h>
#include "config.h"

/* There is no reason in particular why our hashhap has to be the same size as our cache,
 * other than simply not having defined another macro...which we can. We should get rid of
 * the CACHE_SIZE macro towards the end of our implementation after we define a size for 
 * our hashmap and create a function to dynamically choose a size for the cache base upon
 * system RAM.
 */
 
/*=== Per-Inode Dirty List ===*/

typedef struct DL_LL
{
	uint64_t block_number;
	struct DL_LL *next;
} DL_LL;

typedef struct DL_HM_LL
{
	uint64_t inode_number;
	struct DL_LL *list;
	struct DL_HM_LL *next;
} DL_HM_LL;

typedef struct DL_HM
{
	struct DL_HM_LL *HashMap[HASHMAP_SIZE];
} DL_HM;

DL_LL *dl_pop(DL_LL *list);
DL_HM_LL *dl_lookup(DL_HM *hashmap, uint64_t inode_number);
void dl_insert(DL_HM *hashmap, uint64_t Inode_number, uint64_t block_number);
void dl_delete(DL_HM *hashmap, uint64_t inode_number);
void dl_remove_block(DL_HM *hashmap, uint64_t inode_number, uint64_t block_number);

#endif
