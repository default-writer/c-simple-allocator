#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include <stdlib.h>

// Structure for memory blocks with reference counting
typedef struct mem_block {
    void* ptr;              // Pointer to the actual memory
    size_t size;            // Size of the allocated memory
    int ref_count;          // Reference count
    struct mem_block* next; // Next block in the linked list
} mem_block_t;

// Structure for allocator state
typedef struct rc_allocator {
    mem_block_t* block_list;  // List of allocated memory blocks
    int total_blocks;         // Total number of currently allocated blocks
} rc_allocator_t;

// Function declarations for our reference counting allocator
void rc_init(rc_allocator_t* allocator);
void* rc_alloc(rc_allocator_t* allocator, size_t size);
void* rc_retain(rc_allocator_t* allocator, void* ptr);
void rc_release(rc_allocator_t* allocator, void* ptr);

#endif // FUNC_H