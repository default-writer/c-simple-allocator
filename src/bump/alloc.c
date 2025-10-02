#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 4096 // 4KB

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "../api/alloc.h"
#include "../alloc.h"

static allocator_ptr_t _init(void);
static sp_ptr_t _alloc(const allocator_ptr_t* ptr, size_t size);
static void* _retain(const sp_ptr_t* ptr);
static void _release(const sp_ptr_t* ptr);
static void _gc(const allocator_ptr_t* ptr);
static void _destroy(const allocator_ptr_t* ptr);

static alloc_t reference_counting_allocator = {
    .init = _init,
    .alloc = _alloc,
    .retain = _retain,
    .release = _release,
    .gc = _gc,
    .destroy = _destroy
};

static void* memory_block = NULL;
static size_t memory_offset = 0;

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

allocator_ptr_t _init(void) {
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
    memory_offset = 0;
    allocator_t* allocator = _malloc(sizeof(allocator_t));
    if (!allocator) return NULL;
    allocator->block_list = NULL;
    allocator->total_blocks = 0;
    return allocator;
}

sp_ptr_t _alloc(const allocator_ptr_t* ptr, size_t size) {
    if (!ptr || !(*ptr)) return NULL;
    void* _ptr = _malloc(size);
    if (!_ptr) {
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
    allocator_t* _allocator = (allocator_t*)(*ptr);
    smart_pointer->self = (sp_ptr_t)smart_pointer;
    smart_pointer->ref_count = 1;
    smart_pointer->size = size;
    smart_pointer->ptr = _ptr;
    smart_pointer->allocator = _allocator;
    smart_pointer->block = block;
    block->ptr = smart_pointer;
    block->next = (*ptr)->block_list;
    block->prev = NULL;
    if ((*ptr)->block_list != NULL) {
        (*ptr)->block_list->prev = block;
    }
    _allocator->block_list = block;
    _allocator->total_blocks++;
    return smart_pointer;
}

void* _retain(const sp_ptr_t* sp) {
    if (!sp || !(*sp) || (*sp)->self != (sp_ptr_t)*sp) return NULL;
    sp_t* ptr = (sp_t*)*sp;
    ptr->ref_count++;
    return ptr->ptr;
}

void _release(const sp_ptr_t* sp) {
    if (!sp || !(*sp) || (*sp)->self != (sp_ptr_t)*sp) return;
    sp_ptr_t* sp_ptr = (sp_ptr_t*)sp;
    sp_t* ptr = (sp_t*)(*sp);
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
        *sp_ptr = NULL;
    }
}

void _gc(const allocator_ptr_t* ptr) {
    if (!ptr || !(*ptr) || (*ptr)->block_list == NULL || (*ptr)->total_blocks == 0) return;
    allocator_t* allocator = (allocator_t*)(*ptr);
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

void _destroy(const allocator_ptr_t* ptr) {
    if (!ptr || !(*ptr)) return;
    allocator_ptr_t* allocator_ptr = (allocator_ptr_t*)ptr;
    //allocator_t* allocator = (allocator_t*)*ptr;
    *allocator_ptr = NULL;
    // free(allocator); // Cannot free from bump allocator
}
