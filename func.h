#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include <stdlib.h>

#define SMART_PTR_TYPE 1

typedef struct mem_block mem_block_t;
typedef struct rc_allocator rc_allocator_t;

typedef struct smart_ptr {
    unsigned char ref_count;
    unsigned char type;
    size_t size;
    void* ptr;
    mem_block_t* block;
} smart_ptr_t;

typedef struct mem_block {
    smart_ptr_t* ptr;
    mem_block_t* next;
} mem_block_t;
    
typedef struct rc_allocator {
    mem_block_t* block_list;
    int total_blocks;
} rc_allocator_t;

void rc_init(rc_allocator_t* allocator);
smart_ptr_t* rc_alloc(rc_allocator_t* allocator, size_t size);
void* rc_retain(smart_ptr_t* ptr);
void rc_release(rc_allocator_t* allocator, smart_ptr_t* ptr);

#endif // FUNC_H