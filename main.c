#include <stdio.h>
#include <string.h>

#include "reference_counting_allocator_api.h"

int main(void) {
    printf("Reference Counting Memory Allocator Demo\n");
    printf("========================================\n\n");
    
    const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
    
    printf("Allocating memory for a string...\n");
    const_ptr_smart_pointer_t str = reference_counting_allocator_api->alloc(const_allocator_ptr, 20);
    if (str) {
        const char *data = "Hello, world!";
        char *ptr = (char*)reference_counting_allocator_api->retain(str);
        strcpy_s(ptr, 20, data);
        printf("Allocated string: %s\n", ptr);
    }
#if DEBUG
    reference_counting_allocator_api->print_statistics(const_allocator_ptr);
#endif    
    printf("\nRetaining the string...\n");
    char* str2 = reference_counting_allocator_api->retain(str);
    if (str2) {
        printf("Retained string: %s\n", str2);
    }
#if DEBUG
    reference_counting_allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nReleasing one reference...\n");
    
    reference_counting_allocator_api->free(const_allocator_ptr, str);
#if DEBUG
    reference_counting_allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nReleasing final reference...\n");
    reference_counting_allocator_api->free(const_allocator_ptr, str);
#if DEBUG
    reference_counting_allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nAllocating memory for an array...\n");
    const_ptr_smart_pointer_t numbers = reference_counting_allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
    if (numbers) {
        int* numbers_ptr = (int*)reference_counting_allocator_api->retain(numbers);
        
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
    reference_counting_allocator_api->print_statistics(const_allocator_ptr);
#endif    
    
    printf("\nReleasing array...\n");
    reference_counting_allocator_api->free(const_allocator_ptr, numbers);
#if DEBUG
    reference_counting_allocator_api->print_statistics(const_allocator_ptr);
#endif
    printf("\nDemo completed!\n");
    return 0;
}