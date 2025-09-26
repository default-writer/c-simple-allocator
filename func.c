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
smart_ptr_t* rc_alloc(rc_allocator_t* allocator, size_t size) {
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
    
    // Create a new smart pointer
    smart_ptr_t* smart_ptr = malloc(sizeof(smart_ptr_t));
    if (!smart_ptr) {
        free(ptr);
        free(block);
        return NULL;
    }

    // Initialize the smart pointer
    smart_ptr->ref_count = 1;
    smart_ptr->type = SMART_PTR_TYPE;
    smart_ptr->size = size;
    smart_ptr->ptr = ptr;
    smart_ptr->block = block;
    
    // Initialize the block
    block->ptr = smart_ptr;
    // Add to the allocator's list
    block->next = allocator->block_list;
    allocator->block_list = block;
    allocator->total_blocks++;
    
    DEBUG_PRINT("Allocated %zu bytes at %p, ref_count: %d\n", size, ptr, smart_ptr->ref_count);

    return smart_ptr;
}

// Increment reference count
void* rc_retain(smart_ptr_t* ptr) {
    if (!ptr || ptr->type != SMART_PTR_TYPE) return NULL;
    ptr->ref_count++;
    return ptr->ptr;
}

// Decrement reference count and free if needed
void rc_release(rc_allocator_t* allocator, smart_ptr_t* ptr) {
    if (!ptr) return;

    ptr->ref_count--;
    DEBUG_PRINT("Released pointer %p, ref_count: %u\n", ptr, ptr->ref_count);
    
    // If reference count reaches zero, free the memory and remove from list
    if (ptr->ref_count <= 0) {
        DEBUG_PRINT("Freeing %zu bytes at %p\n", ptr->block->size, ptr->ptr);
        
        // Remove block from allocator's block list
        mem_block_t* current = allocator->block_list;
        mem_block_t* previous = NULL;
        
        while (current) {
            if (current->ptr == ptr) {
                // Found the block to remove
                if (previous) {
                    previous->next = current->next;
                } else {
                    // Removing the first block
                    allocator->block_list = current->next;
                }
                allocator->total_blocks--;
                break;
            }
            previous = current;
            current = current->next;
        }
        
        // Free the memory
        free(ptr->ptr);
        free(ptr->block);
        free(ptr);
    }
}