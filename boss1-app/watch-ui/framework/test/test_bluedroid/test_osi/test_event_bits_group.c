/**
 * @file test_event_bits_group.c
 * @brief 这个文件包含osi的thread测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <errno.h>
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/thread.h"
#include "osi/event_bits_group.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static EventBits_t bits = 0;
static void osi_thread_post_handle(void *arg)
{
    EventBitsGroupHandle eg = (EventBitsGroupHandle)arg;

    sleep(5);
    EventBits_t local_bits = eventBitsGroup_getBits(eg);
    eventBitsGroup_setBits(eg, local_bits|1);
}

TEST_GROUP(BT_OSI_EventBitGroup_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_EventBitGroup_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_EventBitGroup_TEST)
{
}

TEST(BT_OSI_EventBitGroup_TEST, eventBitsGroup_create_free)
{
    EventBitsGroupHandle eg = eventBitsGroupCreate();
    TEST_ASSERT_NOT_NULL(eg);

    eventBitsGroup_delete(eg);
}

TEST(BT_OSI_EventBitGroup_TEST, eventBitsGroup_create_op_free)
{
    EventBitsGroupHandle eg = eventBitsGroupCreate();
    TEST_ASSERT_NOT_NULL(eg);

    EventBits_t local_bits = eventBitsGroup_getBits(eg);
    TEST_ASSERT_EQUAL(0, local_bits);

    eventBitsGroup_setBits(eg, local_bits|1);
    local_bits = eventBitsGroup_getBits(eg);
    TEST_ASSERT_EQUAL(1, local_bits);

    eventBitsGroup_clearBits(eg, 1);
    local_bits = eventBitsGroup_getBits(eg);
    TEST_ASSERT_EQUAL(0, local_bits);

    eventBitsGroup_delete(eg);
}

TEST(BT_OSI_EventBitGroup_TEST, eventBitsGroup_create_wait_free)
{
    const size_t work_queue_len[1] = {1};
    osi_thread_t *thread = osi_thread_create("test", 4096, 100, 0, 1, work_queue_len);
    TEST_ASSERT_NOT_NULL(thread);

    EventBitsGroupHandle eg = eventBitsGroupCreate();
    TEST_ASSERT_NOT_NULL(eg);

    bits = eventBitsGroup_getBits(eg);
    TEST_ASSERT_EQUAL(0, bits);

    bits = eventBitsGroup_waitBits(eg, 1, true, false, 1000);
    TEST_ASSERT_EQUAL(0, bits);

    TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, eg, 0, 5000));
    bits = eventBitsGroup_waitBits(eg, 1, true, false, -1);
    TEST_ASSERT_EQUAL(1, bits);

    // clear all bits
    eventBitsGroup_clearBits(eg, 1);
    bits = eventBitsGroup_getBits(eg);
    TEST_ASSERT_EQUAL(0, bits);

    TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, eg, 0, 5000));
    bits = eventBitsGroup_waitBits(eg, 1, true, false, 10000);
    TEST_ASSERT_EQUAL(1, bits);

    {
        // clear all bits
        eventBitsGroup_clearBits(eg, 1);
        bits = eventBitsGroup_getBits(eg);
        TEST_ASSERT_EQUAL(0, bits);

        TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, eg, 0, 5000));
        bits = eventBitsGroup_waitBits(eg, 1, true, true, -1);
        TEST_ASSERT_EQUAL(1, bits);

        bits = eventBitsGroup_getBits(eg);
        TEST_ASSERT_EQUAL(0, bits);
    }

    {
        // clear all bits
        eventBitsGroup_clearBits(eg, 1);
        bits = eventBitsGroup_getBits(eg);
        TEST_ASSERT_EQUAL(0, bits);

        TEST_ASSERT_EQUAL(true, osi_thread_post(thread, osi_thread_post_handle, eg, 0, 5000));
        bits = eventBitsGroup_waitBits(eg, 3, false, true, -1);
        TEST_ASSERT_EQUAL(1, bits);

        bits = eventBitsGroup_getBits(eg);
        TEST_ASSERT_EQUAL(0, bits);
    }

    eventBitsGroup_delete(eg);
    osi_thread_free(thread);
}

TEST_GROUP_RUNNER(BT_OSI_EventBitGroup_TEST)
{
    RUN_TEST_CASE(BT_OSI_EventBitGroup_TEST, eventBitsGroup_create_free);
    RUN_TEST_CASE(BT_OSI_EventBitGroup_TEST, eventBitsGroup_create_op_free);
    RUN_TEST_CASE(BT_OSI_EventBitGroup_TEST, eventBitsGroup_create_wait_free);
}