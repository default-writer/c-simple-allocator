#include <stdio.h>
#include <string.h>

#include "src/api/alloc.h"

#ifdef _WIN32
#define strncpy_s(dest, destsz, src, count) strncpy_s(dest, destsz, src, count)
#else
#define strncpy_s(dest, destsz, src, count) strncpy(dest, src, count); (dest)[(destsz)-1] = '\0';
#endif

int main(void) {
    allocator_ptr_t ptr = alloc->init();
    printf("Allocating memory for a string...\n");
    sp_ptr_t str = alloc->alloc(ptr, 20);
    if (str) {
        const char *data = "Hello, world!";
        char *ptr = (char*)alloc->retain(&str);
        strncpy_s(ptr, 20, data, 20 - 1);
        ptr[20 - 1] = '\0'; // Ensure null-termination
        printf("Allocated string: %s\n", ptr);
    }

    printf("\nRetaining the string...\n");
    char* str2 = alloc->retain(&str);
    if (str2) {
        printf("Retained string: %s\n", str2);
    }
    printf("\nReleasing one reference...\n");
    alloc->release(&str);
    printf("\nReleasing final reference...\n");
    alloc->release(&str);
    printf("\nAllocating memory for an array...\n");
    sp_ptr_t numbers = alloc->alloc(ptr, sizeof(int) * 5);
    if (numbers) {
        int* numbers_ptr = (int*)alloc->retain(&numbers);
        for (int i = 0; i < 5; i++) {
            numbers_ptr[i] = i * 10;
        }
        printf("Allocated array: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", numbers_ptr[i]);
        }
        printf("\n");
    }
    printf("\nReleasing array...\n");
    alloc->release(&numbers);
    alloc->gc(&ptr);
    alloc->destroy(&ptr);

    printf("\nDemo completed!\n");
    return 0;
}