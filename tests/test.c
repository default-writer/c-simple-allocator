#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#define strncpy_s(dest, destsz, src, count) strncpy_s(dest, destsz, src, count)
#else
#define strncpy_s(dest, destsz, src, count) strncpy(dest, src, count); (dest)[(destsz)-1] = '\0';
#endif

#include "../src/api/alloc.h"
#include "../src/alloc.h"

static int tests_run = 0;
static int tests_passed = 0;

const char* GREEN = "";
const char* RED = "";
const char* RESET = "";

void setup_console() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (SetConsoleMode(hOut, dwMode)) {
                GREEN = "\033[0;32m";
                RED = "\033[0;31m";
                RESET = "\033[0m";
            }
        }
    }
#else
    GREEN = "\033[0;32m";
    RED = "\033[0;31m";
    RESET = "\033[0m";
#endif
}

#define TEST(name) \
    do { \
        tests_run++; \
        printf("Running test: %s...", #name); \
        int passed = 1; \
        do

#define END_TEST \
        while (0); \
        if (passed) { \
            tests_passed++; \
            printf("%s  PASSED%s\n", GREEN, RESET); \
        } else { \
            printf("%s  FAILED%s\n", RED, RESET); \
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

#define ASSERT_PTR_NOT_EQ(expected, actual) \
do { \
    if ((expected) == (actual)) { \
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
        const void* result = alloc->retain(NULL);
        ASSERT_PTR_NULL(result);

        allocator_ptr_t ptr = NULL;

        alloc->gc(&ptr);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_retain_valid_pointer_in_allocator() {
    TEST(test_rc_retain_valid_pointer_in_allocator) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t sp = alloc->alloc(&ptr, 10);
        ASSERT_PTR_NOT_NULL(sp);

        mem_block_t* block = (mem_block_t*)ptr->block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_PTR_NULL(block->prev);
        ASSERT_EQ(1, block->ptr->ref_count);

        const void* result = alloc->retain(&sp);
        ASSERT_PTR_EQ(sp->ptr, result);
        ASSERT_EQ(2, block->ptr->ref_count);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_retain_valid_pointer_not_in_allocator() {
    TEST(test_rc_retain_valid_pointer_not_in_allocator) {
        allocator_ptr_t ptr = alloc->init();
        int dummy_value = 42;
        sp_ptr_t sp = (sp_ptr_t)&dummy_value;

        const void* result = alloc->retain(&sp);
        alloc->release(&sp);
        ASSERT_PTR_NULL(result);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_retain_multiple_retains() {
    TEST(test_rc_retain_multiple_retains) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t sp = alloc->alloc(&ptr, 10);
        ASSERT_PTR_NOT_NULL(sp);

        mem_block_t* block = (mem_block_t*)ptr->block_list;
        ASSERT_PTR_NOT_NULL(block);
        ASSERT_PTR_NULL(block->prev);
        ASSERT_EQ(1, block->ptr->ref_count);

        const void* result1 = alloc->retain(&sp);
        const void* result2 = alloc->retain(&sp);
        const void* result3 = alloc->retain(&sp);
        ASSERT_PTR_EQ(sp->ptr, result1);
        ASSERT_PTR_EQ(sp->ptr, result2);
        ASSERT_PTR_EQ(sp->ptr, result3);
        ASSERT_EQ(4, block->ptr->ref_count);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_retain_after_release() {
    TEST(test_rc_retain_after_release) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t sp = alloc->alloc(&ptr, 10);
        ASSERT_PTR_NOT_NULL(sp);

        const void* result1 = alloc->retain(&sp);
        ASSERT_PTR_EQ(sp->ptr, result1);

        mem_block_t* block = (mem_block_t*)ptr->block_list;
        ASSERT_PTR_NULL(block->prev);
        ASSERT_EQ(2, block->ptr->ref_count);

        alloc->release(&sp);
        ASSERT_EQ(1, block->ptr->ref_count);

        const void* result2 = alloc->retain(&sp);
        ASSERT_PTR_EQ(sp->ptr, result2);
        ASSERT_EQ(2, block->ptr->ref_count);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_basic_allocation_adter_gc() {
    TEST(test_basic_allocation) {
        allocator_ptr_t ptr = alloc->init();

        alloc->gc(&ptr);
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        if (str) {
            const char *data = "Hello, world!";
            char *ptr = (char*)(str->ptr);
            strncpy_s(ptr, 20, data, 19);
            ASSERT_PTR_NOT_NULL(str->ptr);
        }
        
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_NOT_EQ(NULL,  ptr->block_list);
        ASSERT_EQ(1, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_basic_allocation_after_destroy() {
    TEST(test_basic_allocation) {
        allocator_ptr_t ptr = alloc->init();

        alloc->destroy(&ptr);
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NULL(str);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_basic_allocation() {
    TEST(test_basic_allocation) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        if (str) {
            const char *data = "Hello, world!";
            char *ptr = (char*)(str->ptr);
            strncpy_s(ptr, 20, data, 19);
            ASSERT_PTR_NOT_NULL(str->ptr);
        }

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_retain_increment_reference_count() {
    TEST(test_retain_increment_reference_count) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        char* str2 = alloc->retain(&str);
        ASSERT_PTR_NOT_NULL(str2);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_release_one_reference() {
    TEST(test_release_one_reference) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        char* str2 = alloc->retain(&str);
        ASSERT_PTR_NOT_NULL(str2);

        alloc->release(&str);
        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_PTR_NOT_NULL(allocator);
        ASSERT_EQ(1, allocator->total_blocks);
        ASSERT_PTR_NOT_EQ(NULL, allocator->block_list);
        ASSERT_PTR_NULL(allocator->block_list->prev);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_allocate_more_memory() {
    TEST(test_allocate_more_memory) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers_ptr = (int*)alloc->retain(&numbers);
        for (int i = 0; i < 5; i++) {
            numbers_ptr[i] = i * 10;
        }
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(i * 10, numbers_ptr[i]);
        }

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_retain_array() {
    TEST(test_retain_array) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers1 = alloc->retain(&numbers);
        for (int i = 0; i < 5; i++) {
            numbers1[i] = i * 10;
        }
        int* numbers2 = alloc->retain(&numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(i * 10, numbers2[i]);
        }

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_release_all_references_to_string() {
    TEST(test_release_all_references_to_string) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        char* str2 = alloc->retain(&str);
        ASSERT_PTR_NOT_NULL(str2);

        alloc->release(&str);
        alloc->release(&str);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_NULL(ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_release_one_reference_to_array() {
    TEST(test_release_one_reference_to_array) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers2 = alloc->retain(&numbers);
        ASSERT_PTR_NOT_NULL(numbers2);
        alloc->release(&numbers);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_release_final_reference_to_array() {
    TEST(test_release_final_reference_to_array) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(numbers);

        int* numbers2 = alloc->retain(&numbers);
        ASSERT_PTR_NOT_NULL(numbers2);

        alloc->release(&numbers);
        alloc->release(&numbers);

        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_PTR_NOT_NULL(allocator);
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_release_already_freed_memory() {
    TEST(test_release_already_freed_memory) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        alloc->release(&str);
        alloc->release(&str);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}
void test_rc_gc_no_blocks() {
    TEST(test_rc_gc_no_blocks) {
        allocator_ptr_t ptr = alloc->init();

        alloc->gc(&ptr);
        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_gc_single_block() {
    TEST(test_rc_gc_single_block) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        alloc->gc(&ptr);
        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_gc_free_block_1_of_3() {
    TEST(test_rc_gc_free_block_1_of_3) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str1 = alloc->alloc(&ptr, 20);
        sp_ptr_t str2 = alloc->alloc(&ptr, 30);
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);

        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        alloc->release(&str1);
        ASSERT_EQ(2, ptr->total_blocks);
        ASSERT_PTR_NOT_EQ(NULL, ptr->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_EQ(0, ptr->total_blocks);
        ASSERT_PTR_EQ(NULL, ptr->block_list);

        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_gc_free_block_2_of_3() {
    TEST(test_rc_gc_free_block_2_of_3) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str1 = alloc->alloc(&ptr, 20);
        sp_ptr_t str2 = alloc->alloc(&ptr, 30);
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);

        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        alloc->release(&str2);
        ASSERT_EQ(2, ptr->total_blocks);
        ASSERT_PTR_NOT_EQ(NULL, ptr->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_EQ(0, ptr->total_blocks);
        ASSERT_PTR_EQ(NULL, ptr->block_list);

        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}


void test_rc_gc_free_block_3_of_3() {
    TEST(test_rc_gc_free_block_3_of_3) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str1 = alloc->alloc(&ptr, 20);
        sp_ptr_t str2 = alloc->alloc(&ptr, 30);
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);

        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        alloc->release(&numbers);
        ASSERT_EQ(2, ptr->total_blocks);
        ASSERT_PTR_NOT_EQ(NULL, ptr->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_EQ(0, ptr->total_blocks);
        ASSERT_PTR_EQ(NULL, ptr->block_list);

        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_gc_memory_cleanup() {
    TEST(test_rc_gc_memory_cleanup) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(str);

        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_EQ(1, allocator->total_blocks);
        ASSERT_PTR_NOT_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_rc_gc_free_one_block() {
    TEST(test_rc_gc_multiple_blocks) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str1 = alloc->alloc(&ptr, 20);
        sp_ptr_t str2 = alloc->alloc(&ptr, 30);
        sp_ptr_t numbers = alloc->alloc(&ptr, sizeof(int) * 5);
        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(numbers);

        alloc->retain(&str2);
        alloc->release(&str2);
        alloc->release(&str2);
        alloc->gc(&ptr);

        allocator_t* allocator = (allocator_t*)ptr;
        ASSERT_PTR_NOT_NULL(allocator);
        ASSERT_EQ(0, allocator->total_blocks);
        ASSERT_PTR_EQ(NULL, allocator->block_list);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_double_linked_list_functionality() {
    TEST(test_double_linked_list_functionality) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t str1 = alloc->alloc(&ptr, 20);
        sp_ptr_t str2 = alloc->alloc(&ptr, 30);
        sp_ptr_t str3 = alloc->alloc(&ptr, 40);
        ASSERT_PTR_NOT_NULL(str1);
        ASSERT_PTR_NOT_NULL(str2);
        ASSERT_PTR_NOT_NULL(str3);

        mem_block_t* block1 = (mem_block_t*)ptr->block_list;
        mem_block_t* block2 = (mem_block_t*)block1->next;
        mem_block_t* block3 = (mem_block_t*)block2->next;
        ASSERT_PTR_EQ(str3->block, block1);
        ASSERT_PTR_EQ(str2->block, block2);
        ASSERT_PTR_EQ(str1->block, block3);
        ASSERT_PTR_NULL(block3->next);
        ASSERT_PTR_NULL(block1->prev);
        ASSERT_PTR_EQ(block1, block2->prev);
        ASSERT_PTR_EQ(block2, block3->prev);

        alloc->release(&str2);
        ASSERT_PTR_NULL(block1->prev);
        ASSERT_PTR_EQ(block3, block1->next);
        ASSERT_PTR_EQ(block1, block3->prev);
        ASSERT_PTR_NULL(block3->next);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

void test_retain_after_release() {
    TEST(test_double_linked_list_functionality) {
        allocator_ptr_t ptr = alloc->init();
        sp_ptr_t sp = alloc->alloc(&ptr, 20);
        ASSERT_PTR_NOT_NULL(sp);

        alloc->retain(&sp);
        alloc->release(&sp);
        alloc->release(&sp);
        alloc->retain(&sp);

        alloc->gc(&ptr);
        ASSERT_PTR_NOT_NULL(ptr);
        ASSERT_PTR_EQ(NULL, ptr->block_list);
        ASSERT_EQ(0, ptr->total_blocks);
        alloc->destroy(&ptr);
        ASSERT_PTR_NULL(ptr);
    } END_TEST;
}

int main() {
    setup_console();
    printf("Running unit tests for reference counting allocator\n");
    printf("==========================================\n\n");
    test_rc_retain_null_pointer();
    test_rc_retain_valid_pointer_in_allocator();
    test_rc_retain_valid_pointer_not_in_allocator();
    test_rc_retain_multiple_retains();
    test_rc_retain_after_release();
    test_basic_allocation_adter_gc();
    test_basic_allocation_after_destroy();
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
    test_retain_after_release();

    printf("\n==========================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    if (tests_run == tests_passed) {
        printf("%sAll tests PASSED!%s\n", GREEN, RESET);
        return 0;
    } else {
        printf("%sSome tests FAILED!%s\n", RED, RESET);
        return 1;
    }
}
