#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reference_counting_allocator_api.h"
#include "reference_counting_allocator.h"

static const_ptr_reference_counting_allocator_t rc_init(void);
static const_ptr_smart_pointer_t rc_alloc(const_ptr_reference_counting_allocator_t allocator, size_t size);
static void* rc_retain(const_ptr_smart_pointer_t ptr);
static void rc_free(const_ptr_reference_counting_allocator_t allocator, const_ptr_smart_pointer_t ptr);

#if DEBUG
static void rc_print_statistics(const_ptr_reference_counting_allocator_t const_allocator);
#endif

static reference_counting_allocator_api_t reference_counting_allocator = {
    .init = rc_init,
    .alloc = rc_alloc,
    .retain = rc_retain,
    .free = rc_free,
#if DEBUG
    .print_statistics = rc_print_statistics
#endif
};

const_ptr_reference_counting_allocator_api_t reference_counting_allocator_api = &reference_counting_allocator;

const_ptr_reference_counting_allocator_t rc_init(void) {
    static reference_counting_allocator_t allocator = {NULL, 0};
    return &allocator;
}

const_ptr_smart_pointer_t rc_alloc(const_ptr_reference_counting_allocator_t const_allocator_ptr, size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }
    
    mem_block_t* block = malloc(sizeof(mem_block_t));
    if (!block) {
        free(ptr);
        return NULL;
    }
    
    smart_pointer_t* smart_pointer = malloc(sizeof(smart_pointer_t));
    if (!smart_pointer) {
        free(ptr);
        free(block);
        return NULL;
    }

    smart_pointer->ref_count = 1;
    smart_pointer->type = SMART_PTR_TYPE;
    smart_pointer->size = size;
    smart_pointer->ptr = ptr;
    smart_pointer->block = block;
    
    smart_pointer->block->ptr = (const_ptr_smart_pointer_t)smart_pointer;

    reference_counting_allocator_t* allocator = (reference_counting_allocator_t*)const_allocator_ptr;
    block->next = allocator->block_list;
    allocator->block_list = block;
    allocator->total_blocks++;
    return (const_ptr_smart_pointer_t)smart_pointer;
}

void* rc_retain(const_ptr_smart_pointer_t const_ptr) {
    if (!const_ptr || const_ptr->type != SMART_PTR_TYPE) return NULL;
    smart_pointer_t* ptr = (smart_pointer_t*)const_ptr;
    ptr->ref_count++;
    return ptr->ptr;
}

void rc_free(const_ptr_reference_counting_allocator_t const_allocator_ptr, const_ptr_smart_pointer_t const_ptr) {
    if (!const_ptr) return;
    smart_pointer_t* ptr = (smart_pointer_t*)const_ptr;
    ptr->ref_count--;
    if (ptr->ref_count <= 0) {
        reference_counting_allocator_t* allocator = (reference_counting_allocator_t*)const_allocator_ptr;
        mem_block_t* current = (mem_block_t*)allocator->block_list;
        mem_block_t* previous = NULL;
        while (current) {
            if (current->ptr == (const_ptr_smart_pointer_t)ptr) {
                if (previous) {
                    previous->next = current->next;
                } else {
                    allocator->block_list = (mem_block_t*)current->next;
                }
                allocator->total_blocks--;
                break;
            }
            previous = current;
            current = (mem_block_t*)current->next;
        }
        free(ptr->ptr);
        free(ptr->block);
        free(ptr);
    }
}

#if DEBUG
void rc_print_statistics(const_ptr_reference_counting_allocator_t const_allocator_ptr) {
    printf("\n=== Memory Status ===\n");
    reference_counting_allocator_t* allocator = (reference_counting_allocator_t*)const_allocator_ptr;
    mem_block_t* current = (mem_block_t*)allocator->block_list;
    int count = 0;
    
    while (current) {
        printf("Block %d: %p, size: %zu\n", count++, current->ptr, current->ptr->size);
        current = (mem_block_t*)current->next;
    }
    
    if (count == 0) {
        printf("No allocated blocks\n");
    }
    printf("Total blocks: %d\n", allocator->total_blocks);
    printf("=====================\n\n");
}
#endif
