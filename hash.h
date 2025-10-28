#ifndef HASH_H
#define HASH_H
#include <stdint.h>

/**
 * Hash function utilities for filesystem operations
 */

/**
 * Compute FNV-1a hash of a file path string
 * Used for efficient path lookups and directory operations
 * @param path Null-terminated path string to hash
 * @return 64-bit hash value
 */
uint64_t path_hash(const char *path);

#endif
