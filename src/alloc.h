#ifndef ALLOC_H
#define ALLOC_H

typedef struct mem_block {
    struct sp* ptr;
    struct mem_block* next;
    struct mem_block* prev;
} mem_block_t;

typedef struct allocator {
    mem_block_t* block_list;
    int total_blocks;
} allocator_t;

typedef struct sp {
    void* ptr;
    mem_block_t* block;
    allocator_t* allocator;
    size_t size;
    unsigned long ref_count;
    unsigned long type;
} sp_t;

#endif // ALLOC_H