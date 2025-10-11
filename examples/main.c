#include <stdio.h>
#include <string.h>

#include "../src/api/alloc.h"

#ifdef _WIN32
#define strncpy_s(dest, dest_size, src, count) strncpy_s(dest, dest_size, src, count)
#else
#define strncpy_s(dest, dest_size, src, count) strncpy(dest, src, count); (dest)[(dest_size)-1] = '\0';
#endif

int main(void) {
    allocator_ptr_t ptr = alloc->init();
    printf("allocating memory for a string...\n");
    sp_ptr_t str = alloc->alloc(&ptr, 20);
    if (str) {
        const char *data = "Hello, world!";
        char *ptr = (char*)alloc->retain(&str);
        strncpy_s(ptr, 20, data, 20 - 1);
        ptr[20 - 1] = '\0'; // Ensure null-termination
        printf("allocated string: %s\n", ptr);
    }
    printf("\nretaining the string...\n");
    char* str2 = alloc->retain(&str);
    if (str2) {
        printf("retained string: %s\n", str2);
    }
    printf("\nreleasing one reference...\n");
    alloc->release(&str);
    printf("\nreleasing final reference...\n");
    alloc->release(&str);
    printf("\nallocating memory for an array...\n");
    sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);
    if (numbers) {
        int* numbers_ptr = (int*)alloc->retain(&numbers);
        for (int i = 0; i < 5; i++) {
            numbers_ptr[i] = i * 10;
        }
        printf("allocated array: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", numbers_ptr[i]);
        }
        printf("\n");
    }
    printf("\nreleasing array...\n");
    alloc->release(&numbers);
    alloc->gc(&ptr);
    alloc->destroy(&ptr);
    printf("\ndemo completed!\n");
    return 0;
}