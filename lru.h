#ifndef LRU_H
#define LRU_H

typedef struct LRU_List
{
	int64_t index;
	struct LRU_List *next;
	struct LRU_List *prev;
} LRU_List;

#endif
