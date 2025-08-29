/**
 * @file test_osi_sem.c
 * @brief 这个文件包含osi的sem测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <errno.h>
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/semaphore.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_SEM_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_SEM_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_SEM_TEST)
{
}

TEST(BT_OSI_SEM_TEST, osi_sem_new_free)
{
  osi_sem_t sem;
  TEST_ASSERT_EQUAL(0, osi_sem_new(&sem, 0, 1));

  osi_sem_free(&sem);
}

TEST(BT_OSI_SEM_TEST, osi_sem_new_take_timeout)
{
  osi_sem_t sem;
  TEST_ASSERT_EQUAL(0, osi_sem_new(&sem, 0, 0));

  TEST_ASSERT_EQUAL_INT(-EAGAIN, osi_sem_take(sem, 3000));

  osi_sem_free(&sem);
}

TEST(BT_OSI_SEM_TEST, osi_sem_new_give_take_timeout)
{
  osi_sem_t sem;
  TEST_ASSERT_EQUAL(0, osi_sem_new(&sem, 0, 0));

  osi_sem_give(sem);
  TEST_ASSERT_EQUAL_INT(0, osi_sem_take(sem, 3000));

  osi_sem_free(&sem);
}

TEST(BT_OSI_SEM_TEST, osi_sem_new_take)
{
  osi_sem_t sem;
  TEST_ASSERT_EQUAL(0, osi_sem_new(&sem, 0, 0));

  TEST_ASSERT_EQUAL_INT(-EAGAIN, osi_sem_take(sem, 0));

  osi_sem_free(&sem);
  TEST_ASSERT_EQUAL(NULL, sem);
}

TEST_GROUP_RUNNER(BT_OSI_SEM_TEST)
{
  RUN_TEST_CASE(BT_OSI_SEM_TEST, osi_sem_new_free);
  RUN_TEST_CASE(BT_OSI_SEM_TEST, osi_sem_new_take_timeout);
  RUN_TEST_CASE(BT_OSI_SEM_TEST, osi_sem_new_give_take_timeout);
  RUN_TEST_CASE(BT_OSI_SEM_TEST, osi_sem_new_take);
}
