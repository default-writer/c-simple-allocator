#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE (4096 * 100) // 400KB initial memory block

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "../api/alloc.h"
#include "../alloc.h"

static allocator_ptr_t _init(void);
static sp_ptr_t _alloc(allocator_ptr_t ptr, size_t size);
static void* _retain(sp_ptr_t ptr);
static void _release(const sp_ptr_t* sp);
static void _gc(allocator_ptr_t ptr);
static void _destroy(const allocator_ptr_t* ptr);

typedef struct memory_block
{
    void* ptr;
    int size;
} memory_block_t;
 
static alloc_t reference_counting_allocator = {
    .init = _init,
    .alloc = _alloc,
    .retain = _retain,
    .release = _release,
    .gc = _gc,
    .destroy = _destroy
};

alloc_ptr_t alloc = &reference_counting_allocator;

static void* _malloc(size_t size) {
    memory_block_t* memory_block_ptr;
#ifdef _WIN32
    memory_block_ptr = VirtualAlloc(NULL, sizeof(memory_block_t) + size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    memory_block_ptr = mmap(NULL, sizeof(memory_block_t) + size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    memory_block_ptr->size = sizeof(memory_block_t) + size;
    memory_block_ptr->ptr = (void*)(memory_block_ptr + 1);
    return memory_block_ptr->ptr;
}

static void _free(void* ptr) {
    memory_block_t* memory_block_ptr = ((memory_block_t*)ptr - 1);
    #ifdef _WIN32
        VirtualFree(memory_block_ptr, 0, MEM_RELEASE);
    #else
        munmap(memory_block_ptr, memory_block_ptr->size);
    #endif
}

allocator_ptr_t _init(void) {
    allocator_t* allocator = _malloc(sizeof(allocator_t));
    if (!allocator) return NULL;
    allocator->block_list = NULL;
    allocator->total_blocks = 0;
    return allocator;
}

sp_ptr_t _alloc(allocator_ptr_t allocator, size_t size) {
    void* ptr = _malloc(size);
    if (!ptr) {
        return NULL;
    }
    mem_block_t* block = _malloc(sizeof(mem_block_t));
    if (!block) {
        _free(ptr);
        return NULL;
    }
    struct sp* smart_pointer = _malloc(sizeof(struct sp));
    if (!smart_pointer) {
        _free(ptr);
        _free(block);
        return NULL;
    }
    allocator_t* _allocator = (allocator_t*)allocator;
    smart_pointer->ref_count = 1;
    smart_pointer->type = SMART_PTR_TYPE;
    smart_pointer->size = size;
    smart_pointer->ptr = ptr;
    smart_pointer->allocator = _allocator;
    smart_pointer->block = block;
    block->ptr = smart_pointer;
    block->next = _allocator->block_list;
    block->prev = NULL;
    if (_allocator->block_list != NULL) {
        _allocator->block_list->prev = block;
    }
    _allocator->block_list = block;
    _allocator->total_blocks++;
    return smart_pointer;
}

void* _retain(sp_ptr_t sp) {
    if (!sp || sp->type != SMART_PTR_TYPE) return NULL;
    struct sp* ptr = (struct sp*)sp;
    ptr->ref_count++;
    return ptr->ptr;
}

void _release(const sp_ptr_t* sp) {
    if (!sp || !(*sp) || (*sp)->type != SMART_PTR_TYPE) return;
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
            _free(current);
            allocator->total_blocks--;
        }
        _free(ptr->ptr);
        _free(ptr);
        *sp_ptr = NULL;
    }
}

void _gc(allocator_ptr_t ptr) {
    if (!ptr || ptr->block_list == NULL || ptr->total_blocks == 0) return;
    allocator_t* allocator = (allocator_t*)ptr;
    mem_block_t* current = (mem_block_t*)allocator->block_list;
    while (current) {
        mem_block_t* next = (mem_block_t*)current->next;
        struct sp* ptr = (struct sp*)current->ptr;
        _free(ptr->ptr);
        _free(ptr);
        _free(current);
        allocator->total_blocks--;
        current = next;
    }
    allocator->block_list = NULL;
}

void _destroy(const allocator_ptr_t* ptr) {
    if (ptr == NULL || *ptr == NULL) return;
    allocator_ptr_t* allocator_ptr = (allocator_ptr_t*)ptr;
    allocator_t* allocator = (allocator_t*)*ptr;
    *allocator_ptr = NULL;
    _free(allocator);
}
