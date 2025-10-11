#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "../api/alloc.h"
#include "../api/thread.h"

typedef struct thread_sp {
    handle_ptr_t hThreads;
    int thread_num;
    allocator_ptr_t allocator;
    thread_func_ptr_t func;
} thread_sp_t;

static thread_sp_ptr_t _create(thread_func_ptr_t func, int thread_num);
static void _start(const thread_sp_ptr_t* ptr);
static void _join(const thread_sp_ptr_t* ptr);
static void _destroy(const thread_sp_ptr_t* ptr);

static thread_t reference_thread = {
    .create = _create,
    .start = _start,
    .join = _join,
    .destroy = _destroy
};

thread_ptr_t thread = &reference_thread;

thread_sp_ptr_t _create(thread_func_ptr_t func, int thread_num) {
    allocator_ptr_t _allocator = alloc->init();
    thread_sp_t* sp = (thread_sp_t*)malloc(sizeof(thread_sp_t));
    if (!sp) {
        return NULL;
    }
    sp->hThreads = (handle_ptr_t)malloc(sizeof(handle_t) * thread_num);
    if (!sp->hThreads) {
        free(sp);
        return NULL;
    }
    sp->thread_num = thread_num;
    sp->allocator = _allocator;
    sp->func = func;
    return (thread_sp_ptr_t)sp;
}

void _start(const thread_sp_ptr_t* ptr) {
    if (!ptr || !*ptr) return;
    thread_sp_t* sp = (thread_sp_t*)*ptr;
    for (int i = 0; i < sp->thread_num; i++) {
#ifdef _WIN32
        sp->hThreads[i] = CreateThread(NULL, 0, (thread_func_ptr_t)sp->func, &sp->allocator, 0, NULL);
        if (sp->hThreads[i] == NULL) {
            _destroy(ptr);
        }
#else
        if (pthread_create(&sp->hThreads[i], NULL, sp->func, &sp->allocator)) {
            _destroy(ptr);
        }
#endif
    }
}

void _join(const thread_sp_ptr_t* ptr) {
    if (!ptr || !*ptr) return;
    thread_sp_t* sp = (thread_sp_t*)*ptr;
#ifdef _WIN32
    WaitForMultipleObjects(sp->thread_num, sp->hThreads, TRUE, INFINITE);
    for (int i = 0; i < sp->thread_num; i++) {
        CloseHandle(sp->hThreads[i]);
    }
#else
    for (int i = 0; i < sp->thread_num; i++) {
        pthread_join(sp->hThreads[i], NULL);
    }
#endif
}

void _destroy(const thread_sp_ptr_t* ptr) {
    if (!ptr || !*ptr) return;
    thread_sp_t* sp = (thread_sp_t*)*ptr;
    thread_sp_ptr_t* _ptr = (thread_sp_ptr_t*)ptr;
    allocator_ptr_t _allocator = sp->allocator;
    alloc->gc(&_allocator);
    alloc->destroy(&_allocator);
    sp->allocator = NULL;
    sp->func = NULL;
    free(sp->hThreads);
    free(sp);
    *_ptr = NULL;
}
