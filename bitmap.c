#include "bitmap.h"

/**
 * Get the value of a bit at the specified index
 * Uses 64-bit words for efficient access
 */
int bitmap_get(void* bm, int ii) {
	uint64_t* ptr = (uint64_t*)bm;
	ptr = ptr + ( ii / 64 );  // Find the 64-bit word containing our bit
	return (*ptr & ((uint64_t)1 << (ii % 64))) >> (ii % 64);  // Extract the bit
}

/**
 * Set or clear a bit at the specified index
 * Uses bitwise operations for efficient manipulation
 */
void bitmap_put(void* bm, int ii, int vv) {
	uint64_t* ptr = (uint64_t*)bm;
	ptr = ptr + ( ii / 64 );  // Find the 64-bit word containing our bit
	// Clear bit if vv==0, set bit otherwise
	*ptr = (vv==0) ? *ptr & ~((uint64_t)1 << (ii % 64)) : *ptr | ((uint64_t)1 << (ii % 64));
}

/**
 * Print the bitmap for debugging purposes
 * Shows each bit as 0 or 1
 */
void bitmap_print(void* bm, int size) {
	printf("===BITMAP START===\n");
	for (int ii = 0; ii < size; ++ii) {
		printf("%d", bitmap_get(bm, ii));
	}
	printf("\n===BITMAP END===\n");
}

