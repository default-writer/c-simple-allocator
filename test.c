#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "func.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

// Simple test framework
#define TEST(name) \
    do { \
        tests_run++; \
        printf("Running test: %s...\n", #name); \
        int passed = 1; \
        do 

#define END_TEST \
        while (0); \
        if (passed) { \
            tests_passed++; \
            printf("  PASSED\n"); \
        } else { \
            printf("  FAILED\n"); \
        } \
    } while (0)

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("  Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  Assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), (long)(actual)); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_PTR_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  Assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_PTR_NULL(actual) \
    do { \
        if (NULL != (actual)) { \
            printf("  Assertion failed at %s:%d: Expected NULL, got %p\n", __FILE__, __LINE__, (actual)); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_PTR_NOT_NULL(actual) \
    do { \
        if (NULL == (actual)) { \
            printf("  Assertion failed at %s:%d: Expected non-NULL, got NULL\n", __FILE__, __LINE__); \
            passed = 0; \
        } \
    } while (0)

// Test functions
void test_rc_retain_null_pointer() {
    TEST(test_rc_retain_null_pointer) {
        rc_allocator_t allocator;
        rc_init(&allocator);
        
        void* result = rc_retain(NULL);
        
        ASSERT_PTR_NULL(result);
    } END_TEST;
}

void test_rc_retain_valid_pointer_in_allocator() {
    TEST(test_rc_retain_valid_pointer_in_allocator) {
        rc_allocator_t allocator;
        rc_init(&allocator);
        
        // Allocate a block of memory
        smart_ptr_t* ptr = rc_alloc(&allocator, 10);
        ASSERT_PTR_NOT_NULL(ptr);
        
        // Check initial ref_count
        mem_block_t* block = allocator.block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_EQ(1, block->ptr->ref_count);
        
        // Retain the pointer
        void* result = rc_retain(ptr);
        
        // Check that the same pointer is returned
        ASSERT_PTR_EQ(ptr->ptr, result);
        
        // Check that ref_count was incremented
        ASSERT_EQ(2, block->ptr->ref_count);
    } END_TEST;
}

void test_rc_retain_valid_pointer_not_in_allocator() {
    TEST(test_rc_retain_valid_pointer_not_in_allocator) {
        rc_allocator_t allocator;
        rc_init(&allocator);
        
        // Create a pointer that is not in our allocator
        int dummy_value = 42;
        smart_ptr_t* ptr = (smart_ptr_t*)&dummy_value;
        
        // Try to retain this pointer
        void* result = rc_retain(ptr);
        
        // Should return NULL since the pointer is not in our allocator
        ASSERT_PTR_NULL(result);
    } END_TEST;
}

void test_rc_retain_multiple_retains() {
    TEST(test_rc_retain_multiple_retains) {
        rc_allocator_t allocator;
        rc_init(&allocator);
        
        // Allocate a block of memory
        smart_ptr_t* ptr = rc_alloc(&allocator, 10);
        ASSERT_PTR_NOT_NULL(ptr);
        
        // Check initial ref_count
        mem_block_t* block = allocator.block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_EQ(1, block->ptr->ref_count);
        
        // Retain the pointer multiple times
        void* result1 = rc_retain(ptr);
        void* result2 = rc_retain(ptr);
        void* result3 = rc_retain(ptr);
        
        // Check that the same pointer is returned each time
        ASSERT_PTR_EQ(ptr->ptr, result1);
        ASSERT_PTR_EQ(ptr->ptr, result2);
        ASSERT_PTR_EQ(ptr->ptr, result3);
        
        // Check that ref_count was incremented correctly
        ASSERT_EQ(4, block->ptr->ref_count);
    } END_TEST;
}

void test_rc_retain_after_release() {
    TEST(test_rc_retain_after_release) {
        rc_allocator_t allocator;
        rc_init(&allocator);
        
        // Allocate a block of memory
        smart_ptr_t* ptr = rc_alloc(&allocator, 10);
        ASSERT_PTR_NOT_NULL(ptr);
        
        // Retain the pointer
        void* result1 = rc_retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result1);
        
        // Check ref_count
        mem_block_t* block = allocator.block_list;
        ASSERT_EQ(2, block->ptr->ref_count);
        
        // Release one reference
        rc_release(&allocator, ptr);
        
        // Check ref_count
        ASSERT_EQ(1, block->ptr->ref_count);
        
        // Retain again
        void* result2 = rc_retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result2);
        
        // Check ref_count
        ASSERT_EQ(2, block->ptr->ref_count);
    } END_TEST;
}

// Test runner
int main() {
    printf("Running unit tests for rc_retain function\n");
    printf("==========================================\n\n");
    
    test_rc_retain_null_pointer();
    test_rc_retain_valid_pointer_in_allocator();
    test_rc_retain_valid_pointer_not_in_allocator();
    test_rc_retain_multiple_retains();
    test_rc_retain_after_release();
    
    printf("\n==========================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_run == tests_passed) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}