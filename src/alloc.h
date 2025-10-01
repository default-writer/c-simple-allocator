#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>
#include <stddef.h>

#include "api/alloc.h"

typedef struct mem_block {
    struct sp* ptr;
    struct mem_block* next;
    struct mem_block* prev;
} mem_block_t;

typedef struct allocator {
    mem_block_t* block_list;
    int total_blocks;
} allocator_t;

typedef struct sp* sp_ptr;

typedef struct sp {
    sp_ptr_t self;
    void* ptr;
    mem_block_t* block;
    allocator_t* allocator;
    size_t size;
    unsigned long ref_count;
} sp_t;

#endif // ALLOC_H