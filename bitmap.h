#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdio.h>

/**
 * Bitmap operations for managing free/used blocks and inodes
 * Uses 64-bit words for efficient bit manipulation
 */

/**
 * Get the value of a bit at the specified index
 * @param bm Pointer to the bitmap
 * @param ii Bit index to check
 * @return 1 if bit is set, 0 if bit is clear
 */
int bitmap_get(void* bm, int ii);

/**
 * Set or clear a bit at the specified index
 * @param bm Pointer to the bitmap
 * @param ii Bit index to modify
 * @param vv Value to set (0 to clear, non-zero to set)
 */
void bitmap_put(void* bm, int ii, int vv);

/**
 * Print the bitmap for debugging purposes
 * @param bm Pointer to the bitmap
 * @param size Number of bits to print
 */
void bitmap_print(void* bm, int size);

#endif
