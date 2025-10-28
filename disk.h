#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "types.h"
#include "bitmap.h"

#include "cache.h"

// ==================== DISK OPERATIONS ====================

/**
 * Open and memory-map a disk image file
 * @param filename Path to disk image file
 * @return Pointer to DiskInterface or NULL on failure
 */
DiskInterface* disk_open(const char* filename);

/**
 * Close disk interface and free resources
 * @param disk Pointer to DiskInterface to close
 */
void disk_close(DiskInterface* disk);

/**
 * Get pointer to a specific block on disk
 * @param disk Pointer to DiskInterface
 * @param pnum Block number to access
 * @return Pointer to block data
 */
void* disk_get_block(DiskInterface* disk, int pnum);

/**
 * Get pointer to superblock (block 0)
 * @param disk Pointer to DiskInterface
 * @return Pointer to superblock data
 */
void* get_superblock(DiskInterface* disk);

/**
 * Get pointer to block allocation bitmap
 * @param disk Pointer to DiskInterface
 * @return Pointer to block bitmap
 */
void* get_block_bitmap(DiskInterface* disk);

/**
 * Get pointer to inode allocation bitmap
 * @param disk Pointer to DiskInterface
 * @return Pointer to inode bitmap
 */
void* get_inode_bitmap(DiskInterface* disk);

/**
 * Get pointer to start of inode table
 * @param disk Pointer to DiskInterface
 * @return Pointer to inode table
 */
void* get_inode_start(DiskInterface* disk);

/**
 * Allocate a free block from the filesystem
 * @param disk Pointer to DiskInterface
 * @return Block number of allocated block, or -1 if no free blocks
 */
int alloc_page(DiskInterface* disk, cache *cache);

/**
 * Free a previously allocated block
 * @param disk Pointer to DiskInterface
 * @param pnum Block number to free
 */
void free_page(DiskInterface* disk, cache *cache, int pnum);

/**
 * Read a block from disk into buffer
 * @param disk Pointer to DiskInterface
 * @param block_num Block number to read
 * @param buffer Buffer to store block data
 * @return 0 on success, -1 on failure
 */
int disk_read_block(DiskInterface* disk, uint64_t block_num, void* buffer);

/**
 * Write buffer data to a block on disk
 * @param disk Pointer to DiskInterface
 * @param block_num Block number to write
 * @param buffer Buffer containing data to write
 * @return 0 on success, -1 on failure
 */
int disk_write_block(DiskInterface* disk, uint64_t block_num, const void* buffer);

/**
 * Format the disk with a new filesystem
 * @param disk Pointer to DiskInterface
 * @param volume_name Name for the new volume
 * @return 0 on success, -1 on failure
 */
int disk_format(DiskInterface* disk, const char* volume_name);

#endif

