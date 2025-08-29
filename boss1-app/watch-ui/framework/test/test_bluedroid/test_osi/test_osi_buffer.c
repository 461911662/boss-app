/**
 * @file test_osi_buffer.c
 * @brief 这个文件包含osi的buffer测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/buffer.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_BUFFER_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_BUFFER_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_BUFFER_TEST)
{
}

TEST(BT_OSI_BUFFER_TEST, buffer_new_free)
{
    buffer_t *buffer = buffer_new(10);

    TEST_ASSERT_NOT_NULL(buffer);

    buffer_free(buffer);
}

TEST(BT_OSI_BUFFER_TEST, buffer_new_buffer_length_free)
{
    buffer_t *buffer = buffer_new(10);
    TEST_ASSERT_NOT_NULL(buffer);

    TEST_ASSERT_EQUAL(10, buffer_length(buffer));

    buffer_free(buffer);
}

TEST(BT_OSI_BUFFER_TEST, buffer_new_ref_free1)
{
    buffer_t *buffer = buffer_new(10);
    TEST_ASSERT_NOT_NULL(buffer);

    buffer_t *buffer_ref = buffer_new_ref(buffer);
    TEST_ASSERT_NOT_NULL(buffer_ref);

    TEST_ASSERT_EQUAL(buffer_length(buffer), buffer_length(buffer_ref));

    buffer_free(buffer);
    buffer_free(buffer_ref);
}

TEST(BT_OSI_BUFFER_TEST, buffer_new_ref_free2)
{
    buffer_t *buffer = buffer_new(10);
    TEST_ASSERT_NOT_NULL(buffer);

    buffer_t *buffer_ref = buffer_new_ref(buffer);
    TEST_ASSERT_NOT_NULL(buffer_ref);

    TEST_ASSERT_EQUAL(buffer_length(buffer), buffer_length(buffer_ref));

    buffer_free(buffer_ref);
    buffer_free(buffer);
}

TEST(BT_OSI_BUFFER_TEST, buffer_new_buffer_ptr_free)
{
    buffer_t *buffer = buffer_new(10);
    TEST_ASSERT_NOT_NULL(buffer);

    char *data1 = (char *)buffer_ptr(buffer);
    *data1 = 0x5a;

    char *data2 = (char *)buffer_ptr(buffer);
    TEST_ASSERT_EQUAL(0x5a, *data2);

    buffer_free(buffer);
}

TEST_GROUP_RUNNER(BT_OSI_BUFFER_TEST)
{
  RUN_TEST_CASE(BT_OSI_BUFFER_TEST, buffer_new_free);
  RUN_TEST_CASE(BT_OSI_BUFFER_TEST, buffer_new_buffer_length_free);
  RUN_TEST_CASE(BT_OSI_BUFFER_TEST, buffer_new_ref_free1);
  RUN_TEST_CASE(BT_OSI_BUFFER_TEST, buffer_new_ref_free2);
  RUN_TEST_CASE(BT_OSI_BUFFER_TEST, buffer_new_buffer_ptr_free);
}
