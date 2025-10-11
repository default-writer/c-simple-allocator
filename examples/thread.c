#include <stdio.h>

#include "../src/api/alloc.h"
#include "../src/api/thread.h"

#define NUM_THREADS 4
#define ALLOC_SIZE 1024

thread_func_result thread_func(void* param) {
    (void)param;
    allocator_ptr_t* _ptr = (allocator_ptr_t*)param;
    sp_ptr_t mem;
    if (!_ptr || !_ptr || !(mem = alloc->alloc(_ptr, ALLOC_SIZE))) {
        printf("thread %lu: failed to allocate memory.\n", get_thread_id());
        return (thread_func_result)1;
    }
    printf("thread 0x%lu: allocated %d bytes.\n", get_thread_id(), ALLOC_SIZE);
    char* data = (char*)alloc->retain(&mem);
    if (data) {
        for (int i = 0; i < ALLOC_SIZE; i++) {
            data[i] = (char)(i % 256);
        }
        printf("thread 0x%lu: wrote to memory.\n", get_thread_id());
    }
    alloc->release(&mem);
    printf("thread 0x%lu: released memory.\n", get_thread_id());
    return 0;
}

int main() {
    thread_sp_ptr_t _ptr = thread->create(thread_func, NUM_THREADS);
    thread->start(&_ptr);
    thread->join(&_ptr);
    printf("\nmulti-threaded demo completed!\n");
    return 0;
}