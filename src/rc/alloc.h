#ifndef REFERENCE_COUNTING_ALLOCATOR_H
#define REFERENCE_COUNTING_ALLOCATOR_H

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
    unsigned long ref_count;
    unsigned long type;
    size_t size;
    void* ptr;
    mem_block_t* block;
    allocator_t* allocator;
} sp_t;


#endif // REFERENCE_COUNTING_ALLOCATOR_H