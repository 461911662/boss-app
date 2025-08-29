/**
 * @file test_osi_fixed_pkt_queue.c
 * @brief 这个文件包含所有的osi fixed_pkt_queue测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "bt_osi_mem.h"
#include "osi/fixed_pkt_queue.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static int g_test_data[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
};

static void test_fixed_pkt_queue_free_cb(pkt_linked_item_t *data)
{
    osi_free(data);
}

static void test_fixed_pkt_queue_cb(fixed_pkt_queue_t *queue)
{
    pkt_linked_item_t *item = fixed_pkt_queue_try_peek_first(queue);
    int *pdata = (int *)item->data;
    TEST_ASSERT_EQUAL(1, pdata[0]);
    TEST_ASSERT_EQUAL(2, pdata[1]);
    TEST_ASSERT_EQUAL(3, pdata[2]);
    TEST_ASSERT_EQUAL(4, pdata[3]);
    TEST_ASSERT_EQUAL(5, pdata[4]);
    TEST_ASSERT_EQUAL(6, pdata[5]);
    TEST_ASSERT_EQUAL(7, pdata[6]);
    TEST_ASSERT_EQUAL(8, pdata[7]);
    TEST_ASSERT_EQUAL(9, pdata[8]);
    TEST_ASSERT_EQUAL(10, pdata[9]);
}

TEST_GROUP(BT_OSI_FIXED_PKT_QUEUE_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_FIXED_PKT_QUEUE_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_FIXED_PKT_QUEUE_TEST)
{
}

TEST(BT_OSI_FIXED_PKT_QUEUE_TEST, fixed_pkt_queue_new_free)
{
    fixed_pkt_queue_t *queue = fixed_pkt_queue_new(10);
    TEST_ASSERT_NOT_NULL(queue);

    TEST_ASSERT_EQUAL(true, fixed_pkt_queue_is_empty(queue));
    TEST_ASSERT_EQUAL(0, fixed_pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(10, fixed_pkt_queue_capacity(queue));
    TEST_ASSERT_EQUAL(NULL, fixed_pkt_queue_try_peek_first(queue));

    fixed_pkt_queue_free(queue, NULL);
}

TEST(BT_OSI_FIXED_PKT_QUEUE_TEST, fixed_pkt_queue_new_op_free)
{
    fixed_pkt_queue_t *queue = fixed_pkt_queue_new(10);
    TEST_ASSERT_NOT_NULL(queue);

    pkt_linked_item_t *item = (pkt_linked_item_t *)osi_malloc(sizeof(pkt_linked_item_t)+sizeof(g_test_data));
    TEST_ASSERT_NOT_NULL(item);
    memcpy(item->data, g_test_data, sizeof(g_test_data));

    TEST_ASSERT_EQUAL(true, fixed_pkt_queue_enqueue(queue, item, FIXED_PKT_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(1, fixed_pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(item, fixed_pkt_queue_try_peek_first(queue));
    TEST_ASSERT_EQUAL(false, fixed_pkt_queue_is_empty(queue));

    fixed_pkt_queue_register_dequeue(queue, test_fixed_pkt_queue_cb);

    fixed_pkt_queue_process(queue);
    TEST_ASSERT_EQUAL(false, fixed_pkt_queue_is_empty(queue));
    TEST_ASSERT_EQUAL(1, fixed_pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(item, fixed_pkt_queue_try_peek_first(queue));

    fixed_pkt_queue_free(queue, test_fixed_pkt_queue_free_cb);
}

TEST_GROUP_RUNNER(BT_OSI_FIXED_PKT_QUEUE_TEST)
{
    RUN_TEST_CASE(BT_OSI_FIXED_PKT_QUEUE_TEST, fixed_pkt_queue_new_free);
    RUN_TEST_CASE(BT_OSI_FIXED_PKT_QUEUE_TEST, fixed_pkt_queue_new_op_free);
}
