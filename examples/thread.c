#include <stdatomic.h>
#include <stdio.h>
#include "../src/api/thread.h"

#define NUM_THREADS 10
#define COUNTER 1000000

thread_func_result thread_func(void* param) {
    (void)param;
    atomic_ulong *p = (atomic_ulong*)param;
    for (int i = 0; i < COUNTER; i++) {
        atomic_fetch_add(p, 1);
    }
    return (thread_func_result)*p;
}

int main() {
    atomic_ulong p = 0;
    thread_sp_ptr_t _ptr = thread->create(thread_func, &p, NUM_THREADS);
    thread->start(&_ptr);
    thread->join(&_ptr);
    printf("final counter value %ld\n", p);
    thread->destroy(&_ptr);
    return 0;
}