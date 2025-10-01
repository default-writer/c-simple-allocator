#ifndef API_ALLOC_H
#define API_ALLOC_H

#include <stdlib.h>

#define SMART_PTR_TYPE 1

typedef const struct sp* sp_ptr_t;
typedef const struct allocator* allocator_ptr_t;
typedef const struct alloc* alloc_ptr_t;

typedef struct alloc
{
    allocator_ptr_t (*init)(void);
    sp_ptr_t (*alloc)(allocator_ptr_t ptr, size_t size);
    void* (*retain)(const sp_ptr_t* pts);
    void (*release)(const sp_ptr_t* ptr);
    void (*gc)(const allocator_ptr_t* ptr);
    void (*destroy)(const allocator_ptr_t* ptr);
} alloc_t;

extern alloc_ptr_t alloc;

#endif // API_ALLOC_H