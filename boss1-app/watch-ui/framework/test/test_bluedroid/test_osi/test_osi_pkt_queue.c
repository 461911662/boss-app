/**
 * @file test_osi_pkt_queue.c
 * @brief 这个文件包含所有的osi pkt_queue测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "bt_osi_mem.h"
#include "osi/pkt_queue.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static int g_test_data[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
};

TEST_GROUP(BT_OSI_PKT_QUEUE_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_PKT_QUEUE_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_PKT_QUEUE_TEST)
{
}

TEST(BT_OSI_PKT_QUEUE_TEST, pkt_queue_new_free)
{
    struct pkt_queue *queue = pkt_queue_create();
    TEST_ASSERT_NOT_NULL(queue);
    pkt_queue_destroy(queue, NULL);

    queue = pkt_queue_create();
    TEST_ASSERT_NOT_NULL(queue);
    pkt_queue_flush(queue, NULL);
    TEST_ASSERT_EQUAL_INT(0, pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(true, pkt_queue_is_empty(queue));
    pkt_queue_destroy(queue, NULL);
}

TEST(BT_OSI_PKT_QUEUE_TEST, pkt_queue_new_op_free)
{
    struct pkt_queue *queue = pkt_queue_create();
    TEST_ASSERT_NOT_NULL(queue);

    pkt_linked_item_t *item = pkt_queue_try_peek_first(queue);
    TEST_ASSERT_NULL(item);

    pkt_linked_item_t *item1 = 
        (pkt_linked_item_t *)osi_calloc(1, sizeof(pkt_linked_item_t)+sizeof(g_test_data));
    TEST_ASSERT_NOT_NULL(item1);
    memcpy(item1->data, g_test_data, sizeof(g_test_data));

    TEST_ASSERT_EQUAL_INT(0, pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(true, pkt_queue_enqueue(queue, item1));
    TEST_ASSERT_EQUAL_INT(1, pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(false, pkt_queue_is_empty(queue));
    pkt_linked_item_t *pitem = pkt_queue_try_peek_first(queue);
    TEST_ASSERT_NOT_NULL(pitem);

    int *test_data = (int *)pitem->data;
    TEST_ASSERT_EQUAL_INT(1, test_data[0]);
    TEST_ASSERT_EQUAL_INT(2, test_data[1]);
    TEST_ASSERT_EQUAL_INT(3, test_data[2]);
    TEST_ASSERT_EQUAL_INT(4, test_data[3]);
    TEST_ASSERT_EQUAL_INT(5, test_data[4]);
    TEST_ASSERT_EQUAL_INT(6, test_data[5]);
    TEST_ASSERT_EQUAL_INT(7, test_data[6]);
    TEST_ASSERT_EQUAL_INT(8, test_data[7]);
    TEST_ASSERT_EQUAL_INT(9, test_data[8]);
    TEST_ASSERT_EQUAL_INT(10, test_data[9]);

    item = pkt_queue_dequeue(queue);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL_INT(0, pkt_queue_length(queue));
    TEST_ASSERT_EQUAL(true, pkt_queue_is_empty(queue));
    osi_free(item);

    pkt_queue_destroy(queue, NULL);
}

TEST_GROUP_RUNNER(BT_OSI_PKT_QUEUE_TEST)
{
    RUN_TEST_CASE(BT_OSI_PKT_QUEUE_TEST, pkt_queue_new_free);
    RUN_TEST_CASE(BT_OSI_PKT_QUEUE_TEST, pkt_queue_new_op_free);
}
