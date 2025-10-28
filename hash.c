#include "hash.h"

/**
 * FNV-1a hash function implementation for filesystem path hashing
 * Provides good distribution and is fast for string hashing
 * Used for efficient path lookups in directory structures
 */
uint64_t path_hash(const char *path) {
    uint64_t hash = 0xcbf29ce484222325ULL; // FNV-1a offset basis (64-bit)
    
    // Process each character in the path string
    for (; *path; ++path) {
        hash ^= (uint64_t)(unsigned char)(*path);  // XOR with current byte
        hash *= 0x100000001b3ULL; // Multiply by FNV prime (64-bit)
    }
    
    return hash;
}

