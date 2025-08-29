/**
 * @file test_osi_memory.c
 * @brief 这个文件包含所有的osi memory测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "bt_osi_mem.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_MEM_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_MEM_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_MEM_TEST)
{
}

TEST(BT_OSI_MEM_TEST, malloc_free)
{
    void *p = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(p);
    osi_free(p);
}

TEST(BT_OSI_MEM_TEST, malloc_equal_string_free)
{
    char *pdest = "Hello World";
    void *p = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(p);

    memcpy(p, pdest, 10);
    TEST_ASSERT_EQUAL_MEMORY(pdest, p, 10);

    char first_char = *((char *)p);
    TEST_ASSERT_EQUAL_CHAR('H', first_char);
    osi_free(p);
}

TEST(BT_OSI_MEM_TEST, calloc_free)
{
    void *p = osi_calloc(1, 10);
    TEST_ASSERT_NOT_NULL(p);
    osi_free(p);
}

TEST(BT_OSI_MEM_TEST, calloc_equal_string_free)
{
    char *pdest = "Hello World";
    void *p = osi_calloc(1, 10);
    TEST_ASSERT_NOT_NULL(p);

    memcpy(p, pdest, 10);
    TEST_ASSERT_EQUAL_MEMORY(pdest, p, 10);

    char first_char = *((char *)p);
    TEST_ASSERT_EQUAL_CHAR('H', first_char);
    osi_free(p);
}

TEST_GROUP_RUNNER(BT_OSI_MEM_TEST)
{
  RUN_TEST_CASE(BT_OSI_MEM_TEST, malloc_free);
  RUN_TEST_CASE(BT_OSI_MEM_TEST, malloc_equal_string_free);
  RUN_TEST_CASE(BT_OSI_MEM_TEST, calloc_free);
  RUN_TEST_CASE(BT_OSI_MEM_TEST, calloc_equal_string_free);
}
