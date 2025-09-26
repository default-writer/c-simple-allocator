#ifndef REFERENCE_COUNTING_ALLOCATOR_API_H
#define REFERENCE_COUNTING_ALLOCATOR_API_H

#define SMART_PTR_TYPE 1

typedef const struct smart_pointer* const_ptr_smart_pointer_t;
typedef const struct reference_counting_allocator* const_ptr_reference_counting_allocator_t;
typedef const struct reference_counting_allocator_api* const_ptr_reference_counting_allocator_api_t;

typedef struct reference_counting_allocator_api
{
    const_ptr_reference_counting_allocator_t (*init)(void);
    const_ptr_smart_pointer_t (*alloc)(const_ptr_reference_counting_allocator_t allocator, size_t size);
    void* (*retain)(const_ptr_smart_pointer_t ptr);
    void (*free)(const_ptr_reference_counting_allocator_t allocator, const_ptr_smart_pointer_t ptr);
    void (*gc)(const_ptr_reference_counting_allocator_t allocator);
#if DEBUG    
    void (*print_statistics)(const_ptr_reference_counting_allocator_t const_allocator_ptr);
#endif
} reference_counting_allocator_api_t;

extern const_ptr_reference_counting_allocator_api_t reference_counting_allocator_api;

#endif // REFERENCE_COUNTING_ALLOCATOR_API_H