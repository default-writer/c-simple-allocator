#ifndef REFERENCE_COUNTING_ALLOCATOR_H
#define REFERENCE_COUNTING_ALLOCATOR_H

typedef const struct mem_block* const_ptr_mem_block_t;
typedef const struct smart_pointer* const_ptr_smart_pointer_t;
typedef const struct smart_pointer* const_ptr_smart_pointer_t;

typedef struct mem_block {
    const_ptr_smart_pointer_t ptr;
    const_ptr_mem_block_t next;
} mem_block_t;

typedef struct smart_pointer {
    unsigned long ref_count;
    unsigned long type;
    size_t size;
    void* ptr;
    mem_block_t* block;
} smart_pointer_t;

typedef struct reference_counting_allocator {
    mem_block_t* block_list;
    int total_blocks;
} reference_counting_allocator_t;

#endif // REFERENCE_COUNTING_ALLOCATOR_H