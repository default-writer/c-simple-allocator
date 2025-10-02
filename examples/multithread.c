#include <stdio.h>
#include <pthread.h>
#include "../src/api/alloc.h"

#define NUM_THREADS 4
#define ALLOC_SIZE 1024

#ifdef _WIN32
#define get_thread_id  GetCurrentThreadId
#else
#define get_thread_id  pthread_self
#endif

#ifdef _WIN32
#include <windows.h>
#endif
// Thread function
#ifdef _WIN32
DWORD WINAPI ThreadFunc(LPVOID lpParam) {
#else    
void* ThreadFunc(void* lpParam) {
#endif
    allocator_ptr_t* ptr = (allocator_ptr_t*)lpParam;
    
    // Allocate memory
    sp_ptr_t mem = alloc->alloc(ptr, ALLOC_SIZE);
    if (!mem) {
        printf("Thread %lu: Failed to allocate memory.\n", get_thread_id());
        return (void*)1;
    }

    printf("Thread %lu: Allocated %d bytes.\n", get_thread_id(), ALLOC_SIZE);

    // Use the memory
    char* data = (char*)alloc->retain(&mem);
    if (data) {
        for (int i = 0; i < ALLOC_SIZE; i++) {
            data[i] = (char)(i % 256);
        }
        printf("Thread %lu: Wrote to memory.\n", get_thread_id());
    }

    // Release the memory
    alloc->release(&mem);
    printf("Thread %lu: Released memory.\n", get_thread_id());

    return 0;
}

int main() {
    allocator_ptr_t ptr = alloc->init();
    if (!ptr) {
        printf("Failed to initialize allocator.\n");
        return 1;
    }

#ifdef _WIN32
    HANDLE hThreads[NUM_THREADS];
#else
    pthread_t hThreads[NUM_THREADS];
#endif

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
#ifdef _WIN32
        hThreads[i] = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            ThreadFunc,             // thread function name
            &ptr,                   // argument to thread function 
            0,                      // use default creation flags 
            NULL);                  // returns the thread identifier 

        if (hThreads[i] == NULL) {
            printf("CreateThread error: %lu\n", GetLastError());
            return 1;
        }
#else
        if (pthread_create(&hThreads[i], NULL, ThreadFunc, &ptr)) {
            printf("pthread_create error\n");
            return 1;
        }
#endif
    }

    // Wait for all threads to terminate
#ifdef _WIN32
    WaitForMultipleObjects(NUM_THREADS, hThreads, TRUE, INFINITE);
#else
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(hThreads[i], NULL);
    }
#endif

#ifdef _WIN32
    // Close all thread handles
    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(hThreads[i]);
    }
#endif

    alloc->gc(&ptr);
    alloc->destroy(&ptr);

    printf("\nMulti-threaded demo completed!\n");

    return 0;
}