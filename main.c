#include <stdio.h>
#include <stdarg.h> // Required for va_list, va_start, va_arg, va_end
#include "func.h"


// Print current memory status (moved from func.c)
void rc_print_status(rc_allocator_t* allocator) {
    printf("\n=== Memory Status ===\n");
    
    mem_block_t* current = allocator->block_list;
    int count = 0;
    
    while (current) {
        printf("Block %d: %p, size: %zu, ref_count: %d\n", 
               count++, current->ptr, current->size, current->ref_count);
        current = current->next;
    }
    
    if (count == 0) {
        printf("No allocated blocks\n");
    }
    printf("Total blocks: %d\n", allocator->total_blocks);
    printf("=====================\n\n");
}

int main(void) {
    printf("Reference Counting Memory Allocator Test\n");
    printf("========================================\n\n");
    
    // Create and initialize allocator state
    rc_allocator_t allocator;
    rc_init(&allocator);
    
    // Test 1: Basic allocation
    printf("Test 1: Basic allocation\n");
    char* str = (char*)rc_alloc(&allocator, 20);
    if (str) {
        printf("Allocated string: %s\n", str);
    }
    rc_print_status(&allocator);
    
    // Test 2: Retain (increment reference count)
    printf("Test 2: Retain (increment reference count)\n");
    char* str2 = (char*)rc_retain(&allocator, str);
    printf("Retained string: %s\n", str2);
    rc_print_status(&allocator);
    
    // Test 3: Release one reference
    printf("Test 3: Release one reference\n");
    rc_release(&allocator, str);
    rc_print_status(&allocator);
    
    // Test 4: Allocate more memory
    printf("Test 4: Allocate more memory\n");
    int* numbers = (int*)rc_alloc(&allocator, sizeof(int) * 5);
    if (numbers) {
        for (int i = 0; i < 5; i++) {
            numbers[i] = i * 10;
        }
        printf("Allocated array: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", numbers[i]);
        }
        printf("\n");
    }
    rc_print_status(&allocator);
    
    // Test 5: Retain the array
    printf("Test 5: Retain the array\n");
    int* numbers2 = (int*)rc_retain(&allocator, numbers);
    printf("Retained array: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", numbers2[i]);
    }
    printf("\n");
    rc_print_status(&allocator);
    
    // Test 6: Release all references to string
    printf("Test 6: Release all references to string\n");
    rc_release(&allocator, str2);
    rc_print_status(&allocator);
    
    // Test 7: Release one reference to array
    printf("Test 7: Release one reference to array\n");
    rc_release(&allocator, numbers);
    rc_print_status(&allocator);
    
    // Test 8: Release final reference to array
    printf("Test 8: Release final reference to array\n");
    rc_release(&allocator, numbers2);
    rc_print_status(&allocator);
    
    // Test 9: Try to release already freed memory (should show warning)
    printf("Test 9: Try to release already freed memory\n");
    rc_release(&allocator, str);
    rc_print_status(&allocator);
    
    printf("All tests completed!\n");
    return 0;
}