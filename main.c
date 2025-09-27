#include <stdio.h>
#include <string.h>

#include "reference_counting_allocator_api.h"

int main(void) {
    printf("Reference Counting Memory Allocator Demo\n");
    printf("========================================\n\n");
    
    allocator_ptr_t const_allocator_ptr = allocator_api->init();
    
    printf("Allocating memory for a string...\n");
    const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
    if (str) {
        const char *data = "Hello, world!";
        char *ptr = (char*)allocator_api->retain(str);
        strcpy_s(ptr, 20, data);
        printf("Allocated string: %s\n", ptr);
    }
#if DEBUG
    allocator_api->print_statistics(const_allocator_ptr);
#endif    
    printf("\nRetaining the string...\n");
    char* str2 = allocator_api->retain(str);
    if (str2) {
        printf("Retained string: %s\n", str2);
    }
#if DEBUG
    allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nReleasing one reference...\n");
    
    allocator_api->free(str);
#if DEBUG
    allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nReleasing final reference...\n");
    allocator_api->free(str);
#if DEBUG
    allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nAllocating memory for an array...\n");
    const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
    if (numbers) {
        int* numbers_ptr = (int*)allocator_api->retain(numbers);
        for (int i = 0; i < 5; i++) {
            numbers_ptr[i] = i * 10;
        }
        printf("Allocated array: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", numbers_ptr[i]);
        }
        printf("\n");
    }
#if DEBUG
    allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nReleasing array...\n");
    allocator_api->free(numbers);
    allocator_api->gc(const_allocator_ptr);
    allocator_api->destroy(&const_allocator_ptr);

#if DEBUG
    allocator_api->print_statistics(const_allocator_ptr);
#endif
    printf("\nDemo completed!\n");
    return 0;
}