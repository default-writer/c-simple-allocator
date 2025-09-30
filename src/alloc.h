#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

typedef struct mem_block {
    struct sp* ptr;
    struct mem_block* next;
    struct mem_block* prev;
} mem_block_t;

typedef struct allocator {
    mem_block_t* block_list;
    int total_blocks;
} allocator_t;

#define BUCKET_COUNT 9
extern const size_t bucket_sizes[BUCKET_COUNT];

typedef struct free_list_node {
    struct free_list_node* next;
    struct free_list_node* prev;
} free_list_node_t;

typedef struct bucket {
    free_list_node_t* free_list;
    size_t block_size;
} bucket_t;

typedef struct bucket_allocator {
    allocator_t base;
    bucket_t buckets[BUCKET_COUNT];
    void* memory_block;
    size_t memory_offset;
    size_t memory_size;
} bucket_allocator_t;

typedef struct sp {
    void* ptr;
    mem_block_t* block;
    allocator_t* allocator;
    size_t size;
    unsigned long ref_count;
    unsigned long type;
} sp_t;

#endif // ALLOC_H