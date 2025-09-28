#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "../api/alloc.h"
#include "../alloc.h"

#define MEMORY_SIZE 4096 // 4KB

static allocator_ptr_t re_init(void);
static sp_ptr_t rc_alloc(allocator_ptr_t ptr, size_t size);
static void* rc_retain(sp_ptr_t ptr);
static void rc_release(sp_ptr_t sp);
static void rc_gc(allocator_ptr_t ptr);
static void rc_destroy(const allocator_ptr_t* ptr);

static void* memory_block = NULL;
static size_t memory_offset = 0;

static alloc_t reference_counting_allocator = {
    .init = re_init,
    .alloc = rc_alloc,
    .retain = rc_retain,
    .release = rc_release,
    .gc = rc_gc,
    .destroy = rc_destroy
};

alloc_ptr_t alloc = &reference_counting_allocator;

static void* _malloc(size_t size) {
    if (memory_block == NULL) {
        return NULL;
    }
    if (memory_offset + size > MEMORY_SIZE) {
        return NULL;
    }
    void* ptr = (char*)memory_block + memory_offset;
    memory_offset += size;
    return ptr;
}

allocator_ptr_t re_init(void) {
    if (memory_block == NULL) {
#ifdef _WIN32
        memory_block = VirtualAlloc(NULL, MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        memory_block = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
        if (memory_block == NULL) {
            return NULL;
        }
    }
    memset(memory_block, 0, MEMORY_SIZE);
    memory_offset = 0;
    allocator_t* allocator = _malloc(sizeof(allocator_t));
    if (!allocator) return NULL;
    allocator->block_list = NULL;
    allocator->total_blocks = 0;
    return allocator;
}

sp_ptr_t rc_alloc(allocator_ptr_t allocator, size_t size) {
    void* ptr = _malloc(size);
    if (!ptr) {
        return NULL;
    }
    mem_block_t* block = _malloc(sizeof(mem_block_t));
    if (!block) {
        // free(ptr); // Cannot free from bump allocator
        return NULL;
    }
    struct sp* smart_pointer = _malloc(sizeof(struct sp));
    if (!smart_pointer) {
        // free(ptr); // Cannot free from bump allocator
        // free(block); // Cannot free from bump allocator
        return NULL;
    }
    allocator_t* _allocator = (allocator_t*)allocator;
    smart_pointer->ref_count = 1;
    smart_pointer->type = SMART_PTR_TYPE;
    smart_pointer->size = size;
    smart_pointer->ptr = ptr;
    smart_pointer->allocator = _allocator;
    smart_pointer->block = block;
    smart_pointer->block->ptr = smart_pointer;
    block->next = allocator->block_list;
    block->prev = NULL;
    if (allocator->block_list != NULL) {
        allocator->block_list->prev = block;
    }
    _allocator->block_list = block;
    _allocator->total_blocks++;
    return smart_pointer;
}

void* rc_retain(sp_ptr_t sp) {
    if (!sp || sp->type != SMART_PTR_TYPE) return NULL;
    struct sp* ptr = (struct sp*)sp;
    ptr->ref_count++;
    return ptr->ptr;
}

void rc_release(sp_ptr_t sp) {
    if (!sp || sp->type != SMART_PTR_TYPE) return;
    struct sp* ptr = (struct sp*)sp;
    ptr->ref_count--;
    if (ptr->ref_count <= 0) {
        allocator_t* allocator = (allocator_t*)ptr->allocator;
        mem_block_t* current = ptr->block;
        if (current && current->ptr == ptr) {
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                allocator->block_list = current->next;
            }
            // free(current); // Cannot free from bump allocator
            allocator->total_blocks--;
        }
        // free(ptr->ptr); // Cannot free from bump allocator
        // free(ptr); // Cannot free from bump allocator
    }
}

void rc_gc(allocator_ptr_t ptr) {
    if (!ptr || ptr->block_list == NULL || ptr->total_blocks == 0) return;
    allocator_t* allocator = (allocator_t*)ptr;
    mem_block_t* current = (mem_block_t*)allocator->block_list;
    while (current) {
        mem_block_t* next = (mem_block_t*)current->next;
        // struct sp* ptr = (struct sp*)current->ptr;
        // free(ptr->ptr); // Cannot free from bump allocator
        // free(ptr); // Cannot free from bump allocator
        // free(current); // Cannot free from bump allocator
        allocator->total_blocks--;
        current = next;
    }
    allocator->block_list = NULL;
}

void rc_destroy(const allocator_ptr_t* ptr) {
    if (ptr == NULL || *ptr == NULL) return;
    allocator_ptr_t* allocator_ptr = (allocator_ptr_t*)ptr;
    //allocator_t* allocator = (allocator_t*)*ptr;
    *allocator_ptr = NULL;
    // free(allocator); // Cannot free from bump allocator
}
