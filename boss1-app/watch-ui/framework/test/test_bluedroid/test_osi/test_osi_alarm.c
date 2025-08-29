/**
 * @file test_osi_alarm.c
 * @brief 这个文件包含所有的osi alarm测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/alarm.h"
#include "osi/semaphore.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static osi_sem_t sem;
static void test_alarm_cb(void *data)
{
    static count = 0;

    count++;
    if (sem) {
        osi_sem_give(sem);
    }
}

static void test_time_alarm(void *data)
{
    if (sem) {
        osi_sem_give(sem);
    }
}

TEST_GROUP(BT_OSI_ALARM_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_ALARM_TEST)
{
    TEST_ASSERT_EQUAL(0, osi_alarm_create_mux());
    osi_alarm_init();
}

TEST_TEAR_DOWN(BT_OSI_ALARM_TEST)
{
    osi_alarm_deinit();
    TEST_ASSERT_EQUAL(0, osi_alarm_delete_mux());
}

TEST(BT_OSI_ALARM_TEST, alarm_new_free)
{
    osi_alarm_t *alarm = osi_alarm_new("test_alarm", test_alarm_cb, NULL, 0);
    TEST_ASSERT_NOT_NULL(alarm);

    osi_alarm_free(alarm);
}

TEST(BT_OSI_ALARM_TEST, alarm_timer1)
{
    osi_sem_new(&sem, 0, 0);
    static timer_t timer1;
    struct sigevent sigevent;
    memset(&sigevent, 0, sizeof(sigevent));
    sigevent.sigev_notify = SIGEV_THREAD;
    sigevent.sigev_notify_function = (void (*)(union sigval))test_time_alarm;
    sigevent.sigev_value.sival_ptr = NULL;

    TEST_ASSERT_EQUAL(0, timer_create(CLOCK_BOOTTIME, &sigevent, &timer1));

    struct timespec ts;
    struct itimerspec wakeup_time;
    memset(&wakeup_time, 0, sizeof(wakeup_time));
    clock_gettime(CLOCK_BOOTTIME, &ts);
    wakeup_time.it_value.tv_sec = -3;
    wakeup_time.it_value.tv_nsec = ts.tv_nsec;
    TEST_ASSERT_EQUAL(0, timer_settime(timer1, 0, &wakeup_time, NULL));

    TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

    memset(&wakeup_time, 0, sizeof(wakeup_time));
    clock_gettime(CLOCK_BOOTTIME, &ts);
    wakeup_time.it_value.tv_sec = 3;
    wakeup_time.it_value.tv_nsec = ts.tv_nsec;
    TEST_ASSERT_EQUAL(0, timer_settime(timer1, 0, &wakeup_time, NULL));

    TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

    osi_sem_free(&sem);
}

TEST(BT_OSI_ALARM_TEST, alarm_timer2)
{
    osi_sem_new(&sem, 0, 0);
    static timer_t timer1;
    struct sigevent sigevent;
    memset(&sigevent, 0, sizeof(sigevent));
    sigevent.sigev_notify = SIGEV_THREAD;
    sigevent.sigev_notify_function = (void (*)(union sigval))test_time_alarm;
    sigevent.sigev_value.sival_ptr = NULL;

    TEST_ASSERT_EQUAL(0, timer_create(CLOCK_MONOTONIC, &sigevent, &timer1));

    struct timespec ts;
    struct itimerspec wakeup_time;
    memset(&wakeup_time, 0, sizeof(wakeup_time));
    clock_gettime(CLOCK_MONOTONIC, &ts);
    wakeup_time.it_value.tv_sec = -3;
    wakeup_time.it_value.tv_nsec = ts.tv_nsec;
    TEST_ASSERT_EQUAL(0, timer_settime(timer1, 0, &wakeup_time, NULL));

    TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

    memset(&wakeup_time, 0, sizeof(wakeup_time));
    clock_gettime(CLOCK_MONOTONIC, &ts);
    wakeup_time.it_value.tv_sec = 3;
    wakeup_time.it_value.tv_nsec = ts.tv_nsec;
    TEST_ASSERT_EQUAL(0, timer_settime(timer1, 0, &wakeup_time, NULL));

    TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

    osi_sem_free(&sem);
}

TEST(BT_OSI_ALARM_TEST, alarm_new_timeout_free)
{
#if 1
    // alarm->cb(alarm->cb_data);
    // //btc_transfer_context(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
    // replace
    // btc_transfer_context(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
    // to enable alarm test
#else
    osi_alarm_t *alarm = osi_alarm_new("test_alarm", test_alarm_cb, NULL, 0);
    TEST_ASSERT_NOT_NULL(alarm);

    osi_sem_new(&sem, 0, 0);

    TEST_ASSERT_EQUAL(OSI_ALARM_ERR_PASS, osi_alarm_set(alarm, 10));

    TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));

    osi_alarm_free(alarm);
    osi_sem_free(&sem);
#endif
}

TEST(BT_OSI_ALARM_TEST, alarm_new_period_free)
{
#if 1
    // alarm->cb(alarm->cb_data);
    // //btc_transfer_context(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
    // replace
    // btc_transfer_context(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
    // to enable alarm test
#else
    osi_alarm_t *alarm = osi_alarm_new("test_alarm", test_alarm_cb, NULL, 0);
    TEST_ASSERT_NOT_NULL(alarm);

    osi_sem_new(&sem, 0, 0);

    TEST_ASSERT_EQUAL(OSI_ALARM_ERR_PASS, osi_alarm_set_periodic(alarm, 5000));
    for(uint32_t i = 0; i<10; i++) {
        TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));
    }

    osi_alarm_cancel(alarm);
    osi_alarm_free(alarm);
    osi_sem_free(&sem);
#endif
}

TEST(BT_OSI_ALARM_TEST, alarm_new_info_free)
{
#if 1
    // alarm->cb(alarm->cb_data);
    // //btc_transfer_context(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
    // replace
    // btc_transfer_context(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
    // to enable alarm test
#else
    osi_alarm_t *alarm = osi_alarm_new("test_alarm", test_alarm_cb, NULL, 0);
    TEST_ASSERT_NOT_NULL(alarm);

    osi_sem_new(&sem, 0, 0);

    TEST_ASSERT_EQUAL(OSI_ALARM_ERR_PASS, osi_alarm_set(alarm, 2000));

    TEST_ASSERT_EQUAL(true, osi_alarm_is_active(alarm));
    osi_alarm_cancel(alarm);
    TEST_ASSERT_EQUAL(true, osi_alarm_is_active(alarm));
    TEST_ASSERT_EQUAL(-EAGAIN, osi_sem_take(sem, 500));

    TEST_ASSERT_EQUAL(OSI_ALARM_ERR_PASS, osi_alarm_set_periodic(alarm, 6000));
    for(uint32_t i = 0; i<4; i++) {
        TEST_ASSERT_EQUAL(0, osi_sem_take(sem, OSI_SEM_MAX_TIMEOUT));
    }
    osi_alarm_cancel(alarm);

    osi_alarm_free(alarm);
    TEST_ASSERT_EQUAL(false, osi_alarm_is_active(alarm));

    osi_sem_free(&sem);
#endif
}

TEST_GROUP_RUNNER(BT_OSI_ALARM_TEST)
{
    RUN_TEST_CASE(BT_OSI_ALARM_TEST, alarm_timer1);
    RUN_TEST_CASE(BT_OSI_ALARM_TEST, alarm_timer2);
    RUN_TEST_CASE(BT_OSI_ALARM_TEST, alarm_new_free);
    RUN_TEST_CASE(BT_OSI_ALARM_TEST, alarm_new_timeout_free);
    RUN_TEST_CASE(BT_OSI_ALARM_TEST, alarm_new_period_free);
    RUN_TEST_CASE(BT_OSI_ALARM_TEST, alarm_new_info_free);
}
