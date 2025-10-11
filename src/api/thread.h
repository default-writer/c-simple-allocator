#ifndef API_THREAD_H
#define API_THREAD_H

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
typedef DWORD thread_func_result;
typedef thread_func_result WINAPI (*thread_func_ptr_t)(LPVOID);
typedef HANDLE* handle_ptr_t;
typedef HANDLE handle_t;
#define get_thread_id GetCurrentThreadId
#else
#include <sys/mman.h>
#include <pthread.h>
typedef void * thread_func_result;
typedef thread_func_result (*thread_func_ptr_t)(void *);
typedef pthread_t* handle_ptr_t;
typedef pthread_t handle_t;
#define get_thread_id pthread_self
#endif

typedef const struct thread_sp* thread_sp_ptr_t;
typedef const struct allocator* allocator_ptr_t;
typedef const struct thread* thread_ptr_t;

typedef struct thread
{
    thread_sp_ptr_t (*create)(thread_func_ptr_t func, int thread_num);
    void (*start)(const thread_sp_ptr_t* ptr);
    void (*join)(const thread_sp_ptr_t* ptr);
    void (*destroy)(const thread_sp_ptr_t* ptr);
} thread_t;


extern thread_ptr_t thread;

#endif // API_THREAD_H