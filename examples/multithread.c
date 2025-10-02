#include <stdio.h>
#include <windows.h>
#include "../src/api/alloc.h"

#define NUM_THREADS 4
#define ALLOC_SIZE 1024

// Thread function
DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    allocator_ptr_t* ptr = (allocator_ptr_t*)lpParam;
    
    // Allocate memory
    sp_ptr_t mem = alloc->alloc(ptr, ALLOC_SIZE);
    if (!mem) {
        printf("Thread %lu: Failed to allocate memory.\n", GetCurrentThreadId());
        return 1;
    }

    printf("Thread %lu: Allocated %d bytes.\n", GetCurrentThreadId(), ALLOC_SIZE);

    // Use the memory
    char* data = (char*)alloc->retain(&mem);
    if (data) {
        for (int i = 0; i < ALLOC_SIZE; i++) {
            data[i] = (char)(i % 256);
        }
        printf("Thread %lu: Wrote to memory.\n", GetCurrentThreadId());
    }

    // Release the memory
    alloc->release(&mem);
    printf("Thread %lu: Released memory.\n", GetCurrentThreadId());

    return 0;
}

int main() {
    allocator_ptr_t ptr = alloc->init();
    if (!ptr) {
        printf("Failed to initialize allocator.\n");
        return 1;
    }

    HANDLE hThreads[NUM_THREADS];

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
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
    }

    // Wait for all threads to terminate
    WaitForMultipleObjects(NUM_THREADS, hThreads, TRUE, INFINITE);

    // Close all thread handles
    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(hThreads[i]);
    }

    alloc->gc(&ptr);
    alloc->destroy(&ptr);

    printf("\nMulti-threaded demo completed!\n");

    return 0;
}