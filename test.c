#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "reference_counting_allocator_api.h"
#include "reference_counting_allocator.h"

static int tests_run = 0;
static int tests_passed = 0;

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

void test_rc_retain_null_pointer() {
    TEST(test_rc_retain_null_pointer) {
        const void* result = reference_counting_allocator_api->retain(NULL);
        ASSERT_PTR_NULL(result);
    } END_TEST;
}

void test_rc_retain_valid_pointer_in_allocator() {
    TEST(test_rc_retain_valid_pointer_in_allocator) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t ptr = reference_counting_allocator_api->alloc(const_allocator_ptr, 10);
        ASSERT_PTR_NOT_NULL(ptr);
        mem_block_t* block = (mem_block_t*)const_allocator_ptr->block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_EQ(1, block->ptr->ref_count);
        const void* result = reference_counting_allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result);
        ASSERT_EQ(2, block->ptr->ref_count);
    } END_TEST;
}

void test_rc_retain_valid_pointer_not_in_allocator() {
    TEST(test_rc_retain_valid_pointer_not_in_allocator) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        int dummy_value = 42;
        smart_pointer_t* ptr = (smart_pointer_t*)&dummy_value;
        const void* result = reference_counting_allocator_api->retain(ptr);
        reference_counting_allocator_api->free(const_allocator_ptr, ptr);
        ASSERT_PTR_NULL(result);
    } END_TEST;
}

void test_rc_retain_multiple_retains() {
    TEST(test_rc_retain_multiple_retains) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t ptr = reference_counting_allocator_api->alloc(const_allocator_ptr, 10);
        ASSERT_PTR_NOT_NULL(ptr);
        mem_block_t* block = (mem_block_t*)const_allocator_ptr->block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_EQ(1, block->ptr->ref_count);
        const void* result1 = reference_counting_allocator_api->retain(ptr);
        const void* result2 = reference_counting_allocator_api->retain(ptr);
        const void* result3 = reference_counting_allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result1);
        ASSERT_PTR_EQ(ptr->ptr, result2);
        ASSERT_PTR_EQ(ptr->ptr, result3);
        ASSERT_EQ(4, block->ptr->ref_count);
    } END_TEST;
}

void test_rc_retain_after_release() {
    TEST(test_rc_retain_after_release) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t ptr = reference_counting_allocator_api->alloc(const_allocator_ptr, 10);
        ASSERT_PTR_NOT_NULL(ptr);
        const void* result1 = reference_counting_allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result1);
        mem_block_t* block = (mem_block_t*)const_allocator_ptr->block_list;
        ASSERT_EQ(2, block->ptr->ref_count);
        reference_counting_allocator_api->free(const_allocator_ptr, ptr);
        ASSERT_EQ(1, block->ptr->ref_count);
        const void* result2 = reference_counting_allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result2);
        ASSERT_EQ(2, block->ptr->ref_count);
    } END_TEST;
}

void test_basic_allocation() {
    TEST(test_basic_allocation) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t str = reference_counting_allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);
        if (str) {
            const char *data = "Hello, world!";
            strcpy_s((char*)(str->ptr), 20, data);
            ASSERT_PTR_NOT_NULL(str->ptr);
        }
    } END_TEST;
}

void test_retain_increment_reference_count() {
    TEST(test_retain_increment_reference_count) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t str = reference_counting_allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);
        char* str2 = reference_counting_allocator_api->retain(str);
        ASSERT_PTR_NOT_NULL(str2);
    } END_TEST;
}

void test_release_one_reference() {
    TEST(test_release_one_reference) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t str = reference_counting_allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);
        char* str2 = reference_counting_allocator_api->retain(str);
        ASSERT_PTR_NOT_NULL(str2);
        reference_counting_allocator_api->free(const_allocator_ptr, str);
    } END_TEST;
}

void test_allocate_more_memory() {
    TEST(test_allocate_more_memory) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t numbers = reference_counting_allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);
        if (numbers) {
            int* numbers_ptr = (int*)numbers->ptr;
            for (int i = 0; i < 5; i++) {
                numbers_ptr[i] = i * 10;
            }
            for (int i = 0; i < 5; i++) {
                ASSERT_EQ(i * 10, numbers_ptr[i]);
            }
        }
    } END_TEST;
}

void test_retain_array() {
    TEST(test_retain_array) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t numbers = reference_counting_allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);
        int* numbers1 = reference_counting_allocator_api->retain(numbers);
        for (int i = 0; i < 5; i++) {
            numbers1[i] = i * 10;
        }
        int* numbers2 = reference_counting_allocator_api->retain(numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(i * 10, numbers2[i]);
        }
    } END_TEST;
}

void test_release_all_references_to_string() {
    TEST(test_release_all_references_to_string) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t str = reference_counting_allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);
        char* str2 = reference_counting_allocator_api->retain(str);
        ASSERT_PTR_NOT_NULL(str2);
        reference_counting_allocator_api->free(const_allocator_ptr, str);
        reference_counting_allocator_api->free(const_allocator_ptr, str);
    } END_TEST;
}

void test_release_one_reference_to_array() {
    TEST(test_release_one_reference_to_array) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t numbers = reference_counting_allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);
        int* numbers2 = reference_counting_allocator_api->retain(numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        reference_counting_allocator_api->free(const_allocator_ptr, numbers);
    } END_TEST;
}

void test_release_final_reference_to_array() {
    TEST(test_release_final_reference_to_array) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t numbers = reference_counting_allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);
        int* numbers2 = reference_counting_allocator_api->retain(numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        reference_counting_allocator_api->free(const_allocator_ptr, numbers);
        reference_counting_allocator_api->free(const_allocator_ptr, numbers);
    } END_TEST;
}

void test_release_already_freed_memory() {
    TEST(test_release_already_freed_memory) {
        const_ptr_reference_counting_allocator_t const_allocator_ptr = reference_counting_allocator_api->init();
        const_ptr_smart_pointer_t str = reference_counting_allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);
        reference_counting_allocator_api->free(const_allocator_ptr, str);
        reference_counting_allocator_api->free(const_allocator_ptr, str);
    } END_TEST;
}

int main() {
    printf("Running unit tests for rc_retain function\n");
    printf("==========================================\n\n");
    
    test_rc_retain_null_pointer();
    test_rc_retain_valid_pointer_in_allocator();
    test_rc_retain_valid_pointer_not_in_allocator();
    test_rc_retain_multiple_retains();
    test_rc_retain_after_release();
    
    test_basic_allocation();
    test_retain_increment_reference_count();
    test_release_one_reference();
    test_allocate_more_memory();
    test_retain_array();
    test_release_all_references_to_string();
    test_release_one_reference_to_array();
    test_release_final_reference_to_array();
    test_release_already_freed_memory();
    
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