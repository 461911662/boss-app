/**
 * @file test_osi_future.c
 * @brief 这个文件包含osi的future测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/future.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_FUTURE_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_FUTURE_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_FUTURE_TEST)
{
}

TEST(BT_OSI_FUTURE_TEST, future_new_free)
{
  future_t *future = future_new();
  TEST_ASSERT_NOT_NULL(future);

  future_free(future);
}

TEST(BT_OSI_FUTURE_TEST, future_new_ready_await_free)
{
  future_t *future = future_new();
  TEST_ASSERT_NOT_NULL(future);

  future_ready(future, (void *)1);

  int value = (int)future_await(future);
  TEST_ASSERT_EQUAL_INT(1, value);
}

TEST(BT_OSI_FUTURE_TEST, future_new_immediate_await)
{
  future_t *future = future_new_immediate((void *)90);
  TEST_ASSERT_NOT_NULL(future);

  int value = (int)future_await(future);
  TEST_ASSERT_EQUAL_INT(90, value);
}

TEST_GROUP_RUNNER(BT_OSI_FUTURE_TEST)
{
  RUN_TEST_CASE(BT_OSI_FUTURE_TEST, future_new_free);
  RUN_TEST_CASE(BT_OSI_FUTURE_TEST, future_new_ready_await_free);
  RUN_TEST_CASE(BT_OSI_FUTURE_TEST, future_new_immediate_await);
}
