#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include "cache.h"
#include "btr.h"

int main()
{
	DiskInterface* disk = disk_open("my.img");
	
	cache *cache = alloc_cache();
	
	alloc_page(disk, cache);  // Reserve block 0
	BTreeNode *root = btree_node_create(disk, cache, false); 
	
	while (true) {
		printf("Select:\n(1) to insert a key\n(2) to search for a key\n(3) for debug print\n(4) to delete a key\n(5) to simulate sync\n> ");
		int choice, key, value;
		scanf("%d", &choice);
		switch (choice) {
			case 1:
				printf("Key to insert: ");
				scanf("%d", &key);
				printf("Value to insert: ");
				scanf("%d", &value);
				btree_insert(disk, cache,root->block_number, key, value);
				break;
			case 2:
				printf("Key to search: ");
				scanf("%d", &key);
				btree_search(disk, cache,root->block_number, key);
				break;
			case 3:
				btree_print(disk, cache,root->block_number, 1);
				break;
			case 4:
				printf("Key to delete: ");
				scanf("%d", &key);
				btree_delete(disk, cache,root->block_number, key);
				break;
			case 5:
				cache_sync(disk, cache);
				break;
			default:
				free_cache(cache);
				disk_close(disk);
				return 0;  // Exit program
		}
	}
}

