#ifndef API_THREAD_H
#define API_THREAD_H

#include <stdlib.h>

typedef const struct thread_sp* thread_sp_ptr_t;
typedef const struct allocator* allocator_ptr_t;
typedef const struct thread* thread_ptr_t;

#ifdef _WIN32
typedef DWORD WINAPI *(*thread_func_ptr_t)(LPVOID);
typedef HANDLE* handle_ptr_t;
#define get_thread_id  GetCurrentThreadId
#else
typedef void *(*thread_func_ptr_t)(void *);
typedef pthread_t* handle_ptr_t;
#define get_thread_id  pthread_self
#endif

typedef struct thread
{
    thread_sp_ptr_t (*create)(thread_func_ptr_t func, int thread_num);
    void (*start)(const thread_sp_ptr_t* ptr);
    void (*join)(const thread_sp_ptr_t* ptr);
} thread_t;


extern thread_ptr_t thread;

#endif // API_THREAD_H