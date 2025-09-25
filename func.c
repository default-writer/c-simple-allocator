#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"

// Debug print macro - only prints when DEBUG is defined
#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

// Initialize the allocator state
void rc_init(rc_allocator_t* allocator) {
    allocator->block_list = NULL;
    allocator->total_blocks = 0;
}

// Allocate memory with reference counting
void* rc_alloc(rc_allocator_t* allocator, size_t size) {
    // Allocate the actual memory
    void* ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }
    
    // Create a new block descriptor
    mem_block_t* block = malloc(sizeof(mem_block_t));
    if (!block) {
        free(ptr);
        return NULL;
    }
    
    // Initialize the block
    block->ptr = ptr;
    block->size = size;
    block->ref_count = 1;
    
    // Add to the allocator's list
    block->next = allocator->block_list;
    allocator->block_list = block;
    allocator->total_blocks++;
    
    DEBUG_PRINT("Allocated %zu bytes at %p, ref_count: %d\n", size, ptr, block->ref_count);
    return ptr;
}

// Increment reference count
void* rc_retain(rc_allocator_t* allocator, void* ptr) {
    if (!ptr) return NULL;
    
    // Find the block in our list
    mem_block_t* current = allocator->block_list;
    while (current) {
        if (current->ptr == ptr) {
            current->ref_count++;
            DEBUG_PRINT("Retained pointer %p, ref_count: %d\n", ptr, current->ref_count);
            return ptr;
        }
        current = current->next;
    }
    
    // Pointer not found in our allocated blocks
    return NULL;
}

// Decrement reference count and free if needed
void rc_release(rc_allocator_t* allocator, void* ptr) {
    if (!ptr) return;
    
    mem_block_t* current = allocator->block_list;
    mem_block_t* prev = NULL;
    
    // Find the block in our list
    while (current) {
        if (current->ptr == ptr) {
            current->ref_count--;
            DEBUG_PRINT("Released pointer %p, ref_count: %d\n", ptr, current->ref_count);
            
            // If reference count reaches zero, free the memory
            if (current->ref_count <= 0) {
                DEBUG_PRINT("Freeing %zu bytes at %p\n", current->size, ptr);
                free(ptr);
                
                // Remove from the list
                if (prev) {
                    prev->next = current->next;
                } else {
                    allocator->block_list = current->next;
                }
                free(current);
                allocator->total_blocks--;
            }
            return;
        }
        prev = current;
        current = current->next;
    }
    
    // Pointer not found in our allocated blocks
    DEBUG_PRINT("Warning: Attempted to release untracked pointer %p\n", ptr);
}