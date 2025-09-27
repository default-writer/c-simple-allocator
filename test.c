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
        const void* result = allocator_api->retain(NULL);
        ASSERT_PTR_NULL(result);

        allocator_ptr_t const_allocator_ptr = NULL;

        allocator_api->gc(const_allocator_ptr);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_retain_valid_pointer_in_allocator() {
    TEST(test_rc_retain_valid_pointer_in_allocator) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t ptr = allocator_api->alloc(const_allocator_ptr, 10);
        ASSERT_PTR_NOT_NULL(ptr);

        mem_block_t* block = (mem_block_t*)const_allocator_ptr->block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_PTR_NULL(block->prev);
        ASSERT_EQ(1, block->ptr->ref_count);

        const void* result = allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result);
        ASSERT_EQ(2, block->ptr->ref_count);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_retain_valid_pointer_not_in_allocator() {
    TEST(test_rc_retain_valid_pointer_not_in_allocator) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        int dummy_value = 42;
        struct sp* ptr = (struct sp*)&dummy_value;

        const void* result = allocator_api->retain(ptr);
        allocator_api->free(ptr);
        ASSERT_PTR_NULL(result);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_retain_multiple_retains() {
    TEST(test_rc_retain_multiple_retains) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t ptr = allocator_api->alloc(const_allocator_ptr, 10);
        ASSERT_PTR_NOT_NULL(ptr);

        mem_block_t* block = (mem_block_t*)const_allocator_ptr->block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_PTR_NULL(block->prev);
        ASSERT_EQ(1, block->ptr->ref_count);

        const void* result1 = allocator_api->retain(ptr);
        const void* result2 = allocator_api->retain(ptr);
        const void* result3 = allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result1);
        ASSERT_PTR_EQ(ptr->ptr, result2);
        ASSERT_PTR_EQ(ptr->ptr, result3);
        ASSERT_EQ(4, block->ptr->ref_count);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_retain_after_release() {
    TEST(test_rc_retain_after_release) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t ptr = allocator_api->alloc(const_allocator_ptr, 10);
        ASSERT_PTR_NOT_NULL(ptr);

        const void* result1 = allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result1);

        mem_block_t* block = (mem_block_t*)const_allocator_ptr->block_list;
        ASSERT_PTR_NULL(block->prev);
        ASSERT_EQ(2, block->ptr->ref_count);

        allocator_api->free(ptr);
        ASSERT_EQ(1, block->ptr->ref_count);

        const void* result2 = allocator_api->retain(ptr);
        ASSERT_PTR_EQ(ptr->ptr, result2);
        ASSERT_EQ(2, block->ptr->ref_count);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_basic_allocation() {
    TEST(test_basic_allocation) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        if (str) {
            const char *data = "Hello, world!";
            strcpy_s((char*)(str->ptr), 20, data);
            ASSERT_PTR_NOT_NULL(str->ptr);
        }

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_retain_increment_reference_count() {
    TEST(test_retain_increment_reference_count) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        char* str2 = allocator_api->retain(str);
        ASSERT_PTR_NOT_NULL(str2);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_release_one_reference() {
    TEST(test_release_one_reference) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        char* str2 = allocator_api->retain(str);
        ASSERT_PTR_NOT_NULL(str2);

        allocator_api->free(str);
        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_PTR_NOT_NULL(allocator);
        ASSERT_EQ(1, allocator->total_blocks);
        ASSERT(allocator->block_list != NULL);
        ASSERT_PTR_NULL(allocator->block_list->prev);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_allocate_more_memory() {
    TEST(test_allocate_more_memory) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers_ptr = (int*)allocator_api->retain(numbers);
        for (int i = 0; i < 5; i++) {
            numbers_ptr[i] = i * 10;
        }
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(i * 10, numbers_ptr[i]);
        }

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_retain_array() {
    TEST(test_retain_array) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers1 = allocator_api->retain(numbers);
        for (int i = 0; i < 5; i++) {
            numbers1[i] = i * 10;
        }
        int* numbers2 = allocator_api->retain(numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(i * 10, numbers2[i]);
        }

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_release_all_references_to_string() {
    TEST(test_release_all_references_to_string) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        char* str2 = allocator_api->retain(str);
        ASSERT_PTR_NOT_NULL(str2);

        allocator_api->free(str);
        allocator_api->free(str);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_NULL(const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_release_one_reference_to_array() {
    TEST(test_release_one_reference_to_array) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers2 = allocator_api->retain(numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        allocator_api->free(numbers);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_release_final_reference_to_array() {
    TEST(test_release_final_reference_to_array) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers2 = allocator_api->retain(numbers);
        ASSERT_PTR_NOT_NULL(numbers2);

        allocator_api->free(numbers);
        allocator_api->free(numbers);

        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_PTR_NOT_NULL(allocator);
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_release_already_freed_memory() {
    TEST(test_release_already_freed_memory) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        allocator_api->free(str);
        allocator_api->free(str);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}
void test_rc_gc_no_blocks() {
    TEST(test_rc_gc_no_blocks) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();

        allocator_api->gc(const_allocator_ptr);
        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_gc_single_block() {
    TEST(test_rc_gc_single_block) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        allocator_api->gc(const_allocator_ptr);
        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_gc_free_block_1_of_3() {
    TEST(test_rc_gc_free_block_1_of_3) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str1 = allocator_api->alloc(const_allocator_ptr, 20);
        const_sp_ptr_t str2 = allocator_api->alloc(const_allocator_ptr, 30);
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);

        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        allocator_api->free(str1);
        ASSERT_EQ(2, const_allocator_ptr->total_blocks);
        ASSERT(const_allocator_ptr->block_list != NULL);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);

        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_gc_free_block_2_of_3() {
    TEST(test_rc_gc_free_block_2_of_3) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str1 = allocator_api->alloc(const_allocator_ptr, 20);
        const_sp_ptr_t str2 = allocator_api->alloc(const_allocator_ptr, 30);
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);

        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        allocator_api->free(str2);
        ASSERT_EQ(2, const_allocator_ptr->total_blocks);
        ASSERT(const_allocator_ptr->block_list != NULL);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);

        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}


void test_rc_gc_free_block_3_of_3() {
    TEST(test_rc_gc_free_block_3_of_3) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str1 = allocator_api->alloc(const_allocator_ptr, 20);
        const_sp_ptr_t str2 = allocator_api->alloc(const_allocator_ptr, 30);
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);

        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        allocator_api->free(numbers);
        ASSERT_EQ(2, const_allocator_ptr->total_blocks);
        ASSERT(const_allocator_ptr->block_list != NULL);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);

        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_gc_memory_cleanup() {
    TEST(test_rc_gc_memory_cleanup) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str = allocator_api->alloc(const_allocator_ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_EQ(1, allocator->total_blocks);
        ASSERT(allocator->block_list != NULL);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_rc_gc_free_one_block() {
    TEST(test_rc_gc_multiple_blocks) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str1 = allocator_api->alloc(const_allocator_ptr, 20);
        const_sp_ptr_t str2 = allocator_api->alloc(const_allocator_ptr, 30);
        const_sp_ptr_t numbers = allocator_api->alloc(const_allocator_ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        allocator_api->retain(str2);
        allocator_api->free(str2);
        allocator_api->free(str2);
        allocator_api->gc(const_allocator_ptr);

        allocator_t* allocator = (allocator_t*)const_allocator_ptr;
        ASSERT_PTR_NOT_NULL(allocator);
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

void test_double_linked_list_functionality() {
    TEST(test_double_linked_list_functionality) {
        allocator_ptr_t const_allocator_ptr = allocator_api->init();
        const_sp_ptr_t str1 = allocator_api->alloc(const_allocator_ptr, 20);
        const_sp_ptr_t str2 = allocator_api->alloc(const_allocator_ptr, 30);
        const_sp_ptr_t str3 = allocator_api->alloc(const_allocator_ptr, 40);
        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(str3);

        mem_block_t* block1 = (mem_block_t*)const_allocator_ptr->block_list;
        mem_block_t* block2 = (mem_block_t*)block1->next;
        mem_block_t* block3 = (mem_block_t*)block2->next;
        
        ASSERT_PTR_EQ(str3->block, block1);
        ASSERT_PTR_EQ(str2->block, block2);
        ASSERT_PTR_EQ(str1->block, block3);
        ASSERT_PTR_NULL(block3->next);
        
        ASSERT_PTR_NULL(block1->prev);
        ASSERT_PTR_EQ(block1, block2->prev);
        ASSERT_PTR_EQ(block2, block3->prev);

        allocator_api->free(str2);
        
        ASSERT_PTR_NULL(block1->prev);
        ASSERT_PTR_EQ(block3, block1->next);
        ASSERT_PTR_EQ(block1, block3->prev);
        ASSERT_PTR_NULL(block3->next);

        allocator_api->gc(const_allocator_ptr);
        ASSERT_PTR_NOT_NULL(const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr->block_list);
        ASSERT_EQ(0, const_allocator_ptr->total_blocks);
        allocator_api->destroy(&const_allocator_ptr);
        ASSERT_PTR_EQ(NULL, const_allocator_ptr);
    } END_TEST;
}

int main() {
    printf("Running unit tests for reference counting allocator\n");
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

    test_rc_gc_no_blocks();
    test_rc_gc_single_block();
    test_rc_gc_free_block_1_of_3();
    test_rc_gc_free_block_2_of_3();
    test_rc_gc_free_block_3_of_3();
    test_rc_gc_memory_cleanup();
    test_rc_gc_free_one_block();
    test_double_linked_list_functionality();

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