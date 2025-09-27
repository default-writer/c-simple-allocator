#ifndef REFERENCE_COUNTING_ALLOCATOR_API_H
#define REFERENCE_COUNTING_ALLOCATOR_API_H

#define SMART_PTR_TYPE 1

typedef const struct sp* const_sp_ptr_t;
typedef const struct allocator* allocator_ptr_t;
typedef const struct allocator_api* allocator_api_ptr_t;

typedef struct allocator_api
{
    allocator_ptr_t (*init)(void);
    const_sp_ptr_t (*alloc)(allocator_ptr_t const_allocator_ptr, size_t size);
    void* (*retain)(const_sp_ptr_t ptr);
    void (*release)(const_sp_ptr_t const_smart_ptr);
    void (*gc)(allocator_ptr_t const_allocator_ptr);
    void (*destroy)(const allocator_ptr_t* const_allocator_ptr);
} allocator_api_t;

extern allocator_api_ptr_t allocator_api;

#endif // REFERENCE_COUNTING_ALLOCATOR_API_H