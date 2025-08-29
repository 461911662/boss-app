/**
 * @file test_osi_allocator.c
 * @brief 这个文件包osi的allocator测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/allocator.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_ALLOCATOR_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_ALLOCATOR_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_ALLOCATOR_TEST)
{
}

TEST(BT_OSI_ALLOCATOR_TEST, osi_strdup_free)
{
    char *p = "hello world";
    char *p1 = osi_strdup(p);
    TEST_ASSERT_EQUAL_MEMORY(p, p1, strlen(p) + 1);
    osi_free(p1);
}

TEST(BT_OSI_ALLOCATOR_TEST, osi_malloc_func)
{
    char *p = osi_malloc_func(10);
    TEST_ASSERT_NOT_NULL(p);
    osi_free(p);
}

TEST(BT_OSI_ALLOCATOR_TEST, osi_free_func)
{
    char *p = osi_malloc_func(10);
    TEST_ASSERT_NOT_NULL(p);
    osi_free_func(p);
}

TEST_GROUP_RUNNER(BT_OSI_ALLOCATOR_TEST)
{
  RUN_TEST_CASE(BT_OSI_ALLOCATOR_TEST, osi_strdup_free);
  RUN_TEST_CASE(BT_OSI_ALLOCATOR_TEST, osi_malloc_func);
  RUN_TEST_CASE(BT_OSI_ALLOCATOR_TEST, osi_free_func);
}
