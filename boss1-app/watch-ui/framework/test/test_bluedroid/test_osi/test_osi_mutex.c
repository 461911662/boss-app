/**
 * @file test_osi_mutex.c
 * @brief 这个文件包含所有的osi mutex测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/thread.h"
#include "osi/mutex.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static osi_mutex_t mutex;
static void osi_thread_post_handle(void *arg)
{
    osi_mutex_t lmutex = (osi_mutex_t)arg;
    osi_mutex_lock(lmutex, OSI_MUTEX_MAX_TIMEOUT);
    sleep(5);
    osi_mutex_unlock(lmutex);
}

TEST_GROUP(BT_OSI_MUTEX_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_MUTEX_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_MUTEX_TEST)
{
}

TEST(BT_OSI_MUTEX_TEST, mutex_new_free)
{
    TEST_ASSERT_EQUAL(0, osi_mutex_new(&mutex));

    osi_mutex_free(&mutex);
    TEST_ASSERT_EQUAL(NULL, mutex);
}

TEST(BT_OSI_MUTEX_TEST, mutex_new_locktime_free)
{
    TEST_ASSERT_EQUAL(0, osi_mutex_new(&mutex));

    TEST_ASSERT_EQUAL(0, osi_mutex_lock(mutex, 1000));

    TEST_ASSERT_EQUAL(-2, osi_mutex_lock(mutex, 1000));

    osi_mutex_unlock(mutex);

    TEST_ASSERT_EQUAL(0, osi_mutex_lock(mutex, 0));
    osi_mutex_unlock(mutex);
    TEST_ASSERT_EQUAL(0, osi_mutex_lock(mutex, 1000));

    osi_mutex_free(&mutex);
    TEST_ASSERT_EQUAL(NULL, mutex);
}

TEST(BT_OSI_MUTEX_TEST, mutex_new_lock_free)
{
    const size_t work_queue_len[1] = {1};
    osi_thread_t *thread = osi_thread_create("test", 4096, 100, 0, 1, work_queue_len);
    TEST_ASSERT_NOT_NULL(thread);

    TEST_ASSERT_EQUAL(0, osi_mutex_new(&mutex));

    TEST_ASSERT_EQUAL(0, osi_mutex_lock(mutex, OSI_MUTEX_MAX_TIMEOUT));
    osi_mutex_unlock(mutex);

    TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, mutex, 0, 2000));
    sleep(2);
    TEST_ASSERT_EQUAL(0, osi_mutex_lock(mutex, OSI_MUTEX_MAX_TIMEOUT));

    osi_mutex_free(&mutex);
    TEST_ASSERT_EQUAL(NULL, mutex);
    osi_thread_free(thread);
}

TEST(BT_OSI_MUTEX_TEST, mutex_global_test)
{
    TEST_ASSERT_EQUAL(0, osi_mutex_global_init());

    osi_mutex_global_lock();
    TEST_ASSERT_EQUAL(1, osi_mutex_get_recursive_cnt());

    osi_mutex_global_unlock();
    TEST_ASSERT_EQUAL(0, osi_mutex_get_recursive_cnt());

    for (int i = 0; i < 10; i++)
    {
        osi_mutex_global_lock();
    }
    TEST_ASSERT_EQUAL(10, osi_mutex_get_recursive_cnt());
    for (int i = 0; i < 10; i++)
    {
        osi_mutex_global_unlock();
    }
    TEST_ASSERT_EQUAL(0, osi_mutex_get_recursive_cnt());

    osi_mutex_global_deinit();
}

TEST_GROUP_RUNNER(BT_OSI_MUTEX_TEST)
{
    RUN_TEST_CASE(BT_OSI_MUTEX_TEST, mutex_new_free);
    RUN_TEST_CASE(BT_OSI_MUTEX_TEST, mutex_new_lock_free);
    RUN_TEST_CASE(BT_OSI_MUTEX_TEST, mutex_new_locktime_free);
    RUN_TEST_CASE(BT_OSI_MUTEX_TEST, mutex_global_test);
}
