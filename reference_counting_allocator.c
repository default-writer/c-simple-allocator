#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reference_counting_allocator_api.h"
#include "reference_counting_allocator.h"

static allocator_ptr_t rc_init(void);
static const_sp_ptr_t rc_alloc(allocator_ptr_t const_allocator_ptr, size_t size);
static void* rc_retain(const_sp_ptr_t ptr);
static void rc_release(const_sp_ptr_t const_smart_ptr);
static void rc_gc(allocator_ptr_t const_allocator_ptr);
static void rc_destroy(const allocator_ptr_t* const_allocator_ptr);

#if DEBUG
static void rc_print_statistics(allocator_ptr_t const_allocator_ptr);
#endif

static allocator_api_t reference_counting_allocator = {
    .init = rc_init,
    .alloc = rc_alloc,
    .retain = rc_retain,
    .release = rc_release,
    .gc = rc_gc,
    .destroy = rc_destroy
#if DEBUG
    .print_statistics = rc_print_statistics
#endif
};

allocator_api_ptr_t allocator_api = &reference_counting_allocator;

allocator_ptr_t rc_init(void) {
    allocator_t* allocator = malloc(sizeof(allocator_t));
    if (!allocator) return NULL;
    allocator->block_list = NULL;
    allocator->total_blocks = 0;
    return allocator;
}

const_sp_ptr_t rc_alloc(allocator_ptr_t const_allocator_ptr, size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }
    
    mem_block_t* block = malloc(sizeof(mem_block_t));
    if (!block) {
        free(ptr);
        return NULL;
    }
    
    struct sp* smart_pointer = malloc(sizeof(struct sp));
    if (!smart_pointer) {
        free(ptr);
        free(block);
        return NULL;
    }

    allocator_t* allocator = (allocator_t*)const_allocator_ptr;

    smart_pointer->ref_count = 1;
    smart_pointer->type = SMART_PTR_TYPE;
    smart_pointer->size = size;
    smart_pointer->ptr = ptr;
    smart_pointer->allocator = allocator;
    smart_pointer->block = block;
    smart_pointer->block->ptr = smart_pointer;
    
    block->next = allocator->block_list;
    block->prev = NULL;
    if (allocator->block_list != NULL) {
        allocator->block_list->prev = block;
    }
    allocator->block_list = block;
    allocator->total_blocks++;
    return (const_sp_ptr_t)smart_pointer;
}

void* rc_retain(const_sp_ptr_t const_allocator_ptr) {
    if (!const_allocator_ptr || const_allocator_ptr->type != SMART_PTR_TYPE) return NULL;
    struct sp* ptr = (struct sp*)const_allocator_ptr;
    ptr->ref_count++;
    return ptr->ptr;
}

void rc_release(const_sp_ptr_t const_smart_ptr) {
    if (!const_smart_ptr || const_smart_ptr->type != SMART_PTR_TYPE) return;
    struct sp* ptr = (struct sp*)const_smart_ptr;
    ptr->ref_count--;
    if (ptr->ref_count <= 0) {
        allocator_t* allocator = (allocator_t*)ptr->allocator;
        mem_block_t* current = ptr->block;
        
        if (current && current->ptr == (const_sp_ptr_t)ptr) {
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                allocator->block_list = current->next;
            }
            free(current);
            allocator->total_blocks--;
        }
        free(ptr->ptr);
        free(ptr);
    }
}

void rc_gc(allocator_ptr_t const_allocator_ptr) {
    if (!const_allocator_ptr || const_allocator_ptr->block_list == NULL || const_allocator_ptr->total_blocks == 0) return;
    allocator_t* allocator = (allocator_t*)const_allocator_ptr;
    mem_block_t* current = (mem_block_t*)allocator->block_list;
    while (current) {
        mem_block_t* next = (mem_block_t*)current->next;
        struct sp* ptr = (struct sp*)current->ptr;
        free(ptr->ptr);
        free(ptr);
        free(current);
        allocator->total_blocks--;
        current = next;
    }
    allocator->block_list = NULL;
}

void rc_destroy(const allocator_ptr_t* const_allocator_ptr) {
    if (const_allocator_ptr == NULL || *const_allocator_ptr == NULL) return;
    allocator_ptr_t* allocator_ptr = (allocator_ptr_t*)const_allocator_ptr;
    allocator_t* allocator = (allocator_t*)*const_allocator_ptr;
    *allocator_ptr = NULL;
    free(allocator);
}

#if DEBUG
void rc_print_statistics(allocator_ptr_t const_allocator_ptr) {
    if (!const_allocator_ptr || const_allocator_ptr->block_list == NULL || const_allocator_ptr->total_blocks == 0) return;
    printf("\n=== Memory Status ===\n");
    allocator_t* allocator = (allocator_t*)const_allocator_ptr;
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
