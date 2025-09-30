#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "../api/alloc.h"
#include "../alloc.h"

#define MEMORY_SIZE (4096 * 100) // 400KB initial memory block

const size_t bucket_sizes[BUCKET_COUNT] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

static allocator_ptr_t bucket_init(void);
static sp_ptr_t bucket_alloc(allocator_ptr_t ptr, size_t size);
static void* bucket_retain(sp_ptr_t ptr);
static void bucket_release(sp_ptr_t sp);
static void bucket_gc(allocator_ptr_t ptr);
static void bucket_destroy(const allocator_ptr_t* ptr);

static alloc_t reference_counting_allocator = {
    .init = bucket_init,
    .alloc = bucket_alloc,
    .retain = bucket_retain,
    .release = bucket_release,
    .gc = bucket_gc,
    .destroy = bucket_destroy
};

alloc_ptr_t alloc = &reference_counting_allocator;

static int find_bucket_index(size_t size) {
    for (int i = 0; i < BUCKET_COUNT; i++) {
        if (size <= bucket_sizes[i]) {
            return i;
        }
    }
    return -1;
}

static void* _alloc_from_bucket(bucket_allocator_t* allocator, int bucket_index) {
    if (bucket_index < 0 || bucket_index >= BUCKET_COUNT) {
        return NULL;
    }
    bucket_t* bucket = &allocator->buckets[bucket_index];
    void* ptr = NULL;
    if (bucket->free_list != NULL) {
        free_list_node_t* free_node = bucket->free_list;
        bucket->free_list = free_node->next;
        if (bucket->free_list) {
            bucket->free_list->prev = NULL;
        }
        ptr = (void*)free_node;
    } else {
        size_t block_size = bucket->block_size;
        if (allocator->memory_offset + block_size > allocator->memory_size) {
            return NULL;
        }
        ptr = (char*)allocator->memory_block + allocator->memory_offset;
        allocator->memory_offset += block_size;
    }
    return ptr;
}

static void _free_to_bucket(bucket_allocator_t* allocator, int bucket_index, void* ptr) {
    if (bucket_index < 0 || bucket_index >= BUCKET_COUNT || ptr == NULL) {
        return;
    }
    bucket_t* bucket = &allocator->buckets[bucket_index];
    free_list_node_t* node = (free_list_node_t*)ptr;
    node->next = bucket->free_list;
    node->prev = NULL;
    if (bucket->free_list) {
        bucket->free_list->prev = node;
    }
    bucket->free_list = node;
}

allocator_ptr_t bucket_init(void) {
    void* memory_block = NULL;
#ifdef _WIN32
    memory_block = VirtualAlloc(NULL, MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    memory_block = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    if (memory_block == NULL) {
        return NULL;
    }

    bucket_allocator_t* allocator = (bucket_allocator_t*)memory_block;
    allocator->memory_block = memory_block;
    allocator->memory_size = MEMORY_SIZE;
    allocator->memory_offset = sizeof(bucket_allocator_t);
    allocator->base.block_list = NULL;
    allocator->base.total_blocks = 0;

    for (int i = 0; i < BUCKET_COUNT; i++) {
        allocator->buckets[i].free_list = NULL;
        allocator->buckets[i].block_size = bucket_sizes[i];
    }
    return (allocator_ptr_t)&allocator->base;
}

sp_ptr_t bucket_alloc(allocator_ptr_t allocator, size_t size) {
    bucket_allocator_t* bucket_allocator = (bucket_allocator_t*)((char*)allocator - offsetof(bucket_allocator_t, base));
    
    int user_bucket_index = find_bucket_index(size);
    if (user_bucket_index == -1) return NULL;

    int sp_bucket_index = find_bucket_index(sizeof(sp_t));
    if (sp_bucket_index == -1) return NULL;

    int mb_bucket_index = find_bucket_index(sizeof(mem_block_t));
    if (mb_bucket_index == -1) return NULL;

    void* user_ptr = _alloc_from_bucket(bucket_allocator, user_bucket_index);
    if (!user_ptr) return NULL;

    sp_t* smart_pointer = _alloc_from_bucket(bucket_allocator, sp_bucket_index);
    if (!smart_pointer) {
        _free_to_bucket(bucket_allocator, user_bucket_index, user_ptr);
        return NULL;
    }

    mem_block_t* mem_block = _alloc_from_bucket(bucket_allocator, mb_bucket_index);
    if (!mem_block) {
        _free_to_bucket(bucket_allocator, user_bucket_index, user_ptr);
        _free_to_bucket(bucket_allocator, sp_bucket_index, smart_pointer);
        return NULL;
    }

    smart_pointer->ref_count = 1;
    smart_pointer->type = SMART_PTR_TYPE;
    smart_pointer->size = size;
    smart_pointer->ptr = user_ptr;
    smart_pointer->allocator = (allocator_t*)allocator;
    smart_pointer->block = mem_block;

    mem_block->ptr = smart_pointer;
    mem_block->next = bucket_allocator->base.block_list;
    mem_block->prev = NULL;
    if (bucket_allocator->base.block_list != NULL) {
        bucket_allocator->base.block_list->prev = mem_block;
    }
    bucket_allocator->base.block_list = mem_block;
    bucket_allocator->base.total_blocks++;

    return smart_pointer;
}

void bucket_release(sp_ptr_t sp) {
    if (!sp || sp->type != SMART_PTR_TYPE) return;
    
    sp_t* ptr = (sp_t*)sp;
    if (ptr->ref_count == 0) return;
    ptr->ref_count--;

    if (ptr->ref_count <= 0) {
        bucket_allocator_t* allocator = (bucket_allocator_t*)((char*)ptr->allocator - offsetof(bucket_allocator_t, base));
        mem_block_t* current = ptr->block;

        // Update the block list FIRST
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
        if (current->prev != NULL) {
            current->prev->next = current->next;
        } else {
            allocator->base.block_list = current->next;
        }
        allocator->base.total_blocks--;

        // Now free the memory
        int user_bucket_index = find_bucket_index(ptr->size);
        _free_to_bucket(allocator, user_bucket_index, ptr->ptr);

        // int sp_bucket_index = find_bucket_index(sizeof(sp_t));
        // _free_to_bucket(allocator, sp_bucket_index, ptr);

        int mb_bucket_index = find_bucket_index(sizeof(mem_block_t));
        _free_to_bucket(allocator, mb_bucket_index, current);
    }
}

void* bucket_retain(sp_ptr_t sp) {
    if (!sp || sp->type != SMART_PTR_TYPE) return NULL;
    sp_t* ptr = (sp_t*)sp;
    ptr->ref_count++;
    return ptr->ptr;
}

void bucket_gc(allocator_ptr_t ptr) {
    if (!ptr) return;
    bucket_allocator_t* allocator = (bucket_allocator_t*)((char*)ptr - offsetof(bucket_allocator_t, base));
    mem_block_t* current = allocator->base.block_list;
    while (current) {
        mem_block_t* next = current->next;
        
        sp_t* sp = current->ptr;
        
        int user_bucket_index = find_bucket_index(sp->size);
        _free_to_bucket(allocator, user_bucket_index, sp->ptr);

        int sp_bucket_index = find_bucket_index(sizeof(sp_t));
        _free_to_bucket(allocator, sp_bucket_index, sp);

        int mb_bucket_index = find_bucket_index(sizeof(mem_block_t));
        _free_to_bucket(allocator, mb_bucket_index, current);
        
        current = next;
    }
    allocator->base.block_list = NULL;
    allocator->base.total_blocks = 0;
}

void bucket_destroy(const allocator_ptr_t* ptr) {
    if (ptr == NULL || *ptr == NULL) return;
    
    bucket_allocator_t* allocator = (bucket_allocator_t*)((char*)*ptr - offsetof(bucket_allocator_t, base));
#ifdef _WIN32
    VirtualFree(allocator->memory_block, 0, MEM_RELEASE);
#else
    munmap(allocator->memory_block, allocator->memory_size);
#endif
    *((allocator_ptr_t*)ptr) = NULL;
}
