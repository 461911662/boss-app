/**
 * @file test_osi_thread.c
 * @brief 这个文件包含osi的thread测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <errno.h>
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/thread.h"
#include "osi/semaphore.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_THREAD_TEST);

static osi_sem_t sem;
static void osi_thread_post_handle(void *arg)
{
  osi_sem_t *lsem = (osi_sem_t *)arg;
  if (*lsem) {
    (void)osi_sem_give(*lsem);
  }
}

static void osi_thread_postevent_handle(void *arg)
{
  osi_sem_t *lsem = (osi_sem_t *)arg;
  if (*lsem) {
    (void)osi_sem_give(*lsem);
  }
}

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_THREAD_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_THREAD_TEST)
{
}

TEST(BT_OSI_THREAD_TEST, thread_create_free)
{
  const size_t work_queue_len[10] = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
  osi_thread_t *thread = osi_thread_create("test", 4096, 100, 0, 10, work_queue_len);
  TEST_ASSERT_NOT_NULL(thread);

  TEST_ASSERT_EQUAL(0, osi_thread_attr_test(thread, 100));

  osi_thread_free(thread);
}

TEST(BT_OSI_THREAD_TEST, thread_create_post_free)
{
  const size_t work_queue_len[10] = {4096, 0, 1, 4096, 4096, 4096, 4096, 4096, 4096, 4096};
  osi_thread_t *thread = osi_thread_create("test", 4096, 100, 0, 10, work_queue_len);
  TEST_ASSERT_NOT_NULL(thread);

  TEST_ASSERT_EQUAL(0, osi_sem_new(&sem, 0, 0));

  // test normal timeout
  TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, &sem, 0, 2000));
  TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

  // test timeout 0
  TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, &sem, 0, 10));
  TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

  // test queue index 0, as DEFAULT_WORK_QUEUE_CAPACITY
  TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, &sem, 1, 2000));
  TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

  // test queue index 1
  TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, &sem, 2, 2000));
  TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, &sem, 2, OSI_THREAD_MAX_TIMEOUT));
  TEST_ASSERT_EQUAL(false, osi_thread_post(thread, osi_thread_post_handle, &sem, 2, 100));

  // test queue invalid
  TEST_ASSERT_EQUAL(false, osi_thread_post(thread, osi_thread_post_handle, &sem, 10, 2000));

  osi_sem_free(&sem);
  osi_thread_free(thread);
}

TEST(BT_OSI_THREAD_TEST, thread_create_postevent_free)
{
  const size_t work_queue_len[10] = {4096, 0, 4, 4096, 4096, 4096, 4096, 4096, 4096, 4096};
  osi_thread_t *thread = osi_thread_create("test", 4096, 100, 0, 10, work_queue_len);
  TEST_ASSERT_NOT_NULL(thread);

  TEST_ASSERT_EQUAL(0, osi_sem_new(&sem, 0, 0));

  // test event create delete
  struct osi_event *event = osi_event_create(osi_thread_postevent_handle, &sem);
  TEST_ASSERT_NOT_NULL(event);

  // test bind event
  TEST_ASSERT_EQUAL(true, osi_event_bind(event, thread, 0));
  TEST_ASSERT_EQUAL(true, osi_thread_post_event(event, 0));
  osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT);

  // test event timeout 2000
  TEST_ASSERT_EQUAL(true, osi_thread_post_event(event, 2000));
  osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT);

  // test event timeout OSI_THREAD_MAX_TIMEOUT
  TEST_ASSERT_EQUAL(true, osi_thread_post_event(event, OSI_THREAD_MAX_TIMEOUT));
  osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT);

  osi_event_delete(event);
  osi_sem_free(&sem);
  osi_thread_free(thread);
}

TEST_GROUP_RUNNER(BT_OSI_THREAD_TEST)
{
  RUN_TEST_CASE(BT_OSI_THREAD_TEST, thread_create_free);
  RUN_TEST_CASE(BT_OSI_THREAD_TEST, thread_create_post_free);
  RUN_TEST_CASE(BT_OSI_THREAD_TEST, thread_create_postevent_free);
}