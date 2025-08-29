/**
 * @file test_osi_fixed_queue.c
 * @brief 这个文件包含所有的osi fixed_queue测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/fixed_queue.h"
#include "bt_osi_mem.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define CMP_CONSTANT (0xEF)

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static void test_fixed_queue_cb(fixed_queue_t *queue)
{
    uint32_t *data = NULL;
    uint32_t len = fixed_queue_length(queue);
    uint32_t cnt = 0;

    while(fixed_queue_try_peek_first(queue) != NULL) {
        if (cnt++ >= len) {
            break;
        }
        data = (uint32_t *)fixed_queue_dequeue(queue, FIXED_QUEUE_MAX_TIMEOUT);
        if (*data != CMP_CONSTANT) {
            TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data, FIXED_QUEUE_MAX_TIMEOUT));
            continue;
        }

        osi_free(data);
        data = NULL;
    }
}

static void test_fixed_queue_free_cb(void *data)
{
    osi_free(data);
}

TEST_GROUP(BT_OSI_FIXED_QUEUE_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_FIXED_QUEUE_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_FIXED_QUEUE_TEST)
{
}

TEST(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_free)
{
    fixed_queue_t *queue = NULL;
    queue = fixed_queue_new(10);
    TEST_ASSERT_NOT_NULL(queue);

    TEST_ASSERT_EQUAL(true, fixed_queue_is_empty(queue));
    TEST_ASSERT_EQUAL(0, fixed_queue_length(queue));
    TEST_ASSERT_EQUAL(10, fixed_queue_capacity(queue));
    TEST_ASSERT_EQUAL(NULL, fixed_queue_try_peek_first(queue));
    TEST_ASSERT_EQUAL(NULL, fixed_queue_try_peek_last(queue));

    fixed_queue_free(queue, NULL);
}

TEST(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_op1_free)
{
    fixed_queue_t *queue = NULL;
    queue = fixed_queue_new(3);
    TEST_ASSERT_NOT_NULL(queue);

    uint32_t *data1 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data1);
    *data1 = CMP_CONSTANT;

    uint32_t *data2 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data2);
    *data2 = CMP_CONSTANT;

    uint32_t *data3 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data3);
    *data3 = CMP_CONSTANT;

    uint32_t *data4 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data4);
    *data4 = CMP_CONSTANT;

    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data1, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data2, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data3, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(false, fixed_queue_enqueue(queue, data4, 0));

    fixed_queue_register_dequeue(queue, test_fixed_queue_cb);

    fixed_queue_process(queue);

    TEST_ASSERT_EQUAL(NULL, fixed_queue_try_peek_first(queue));
    TEST_ASSERT_EQUAL(NULL, fixed_queue_try_peek_last(queue));

    osi_free(data4);
    fixed_queue_free(queue, test_fixed_queue_free_cb);
}

TEST(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_op2_free)
{
    fixed_queue_t *queue = NULL;
    queue = fixed_queue_new(3);
    TEST_ASSERT_NOT_NULL(queue);

    uint32_t *data1 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data1);
    *data1 = CMP_CONSTANT;

    uint32_t *data2 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data2);
    *data2 = CMP_CONSTANT;

    uint32_t *data3 = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data3);
    *data3 = 0xf2;

    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data1, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(1, fixed_queue_length(queue));
    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data2, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(2, fixed_queue_length(queue));
    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data3, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(3, fixed_queue_length(queue));

    fixed_queue_register_dequeue(queue, test_fixed_queue_cb);

    fixed_queue_process(queue);

    TEST_ASSERT_EQUAL(1, fixed_queue_length(queue));

    fixed_queue_free(queue, test_fixed_queue_free_cb);
}

TEST(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_op3_free)
{
    fixed_queue_t *queue = NULL;
    queue = fixed_queue_new(3);
    TEST_ASSERT_NOT_NULL(queue);

    uint32_t *data = (uint32_t *)osi_malloc(sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(data);
    *data = CMP_CONSTANT;

    TEST_ASSERT_EQUAL(true, fixed_queue_enqueue(queue, data, FIXED_QUEUE_MAX_TIMEOUT));
    TEST_ASSERT_EQUAL(1, fixed_queue_length(queue));

    uint32_t *pdata = fixed_queue_try_remove_from_queue(queue, data);
    TEST_ASSERT_NOT_NULL(pdata);
    test_fixed_queue_free_cb(pdata);
    TEST_ASSERT_EQUAL(0, fixed_queue_length(queue));

    fixed_queue_free(queue, test_fixed_queue_free_cb);
}

TEST_GROUP_RUNNER(BT_OSI_FIXED_QUEUE_TEST)
{
    RUN_TEST_CASE(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_free);
    RUN_TEST_CASE(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_op1_free);
    RUN_TEST_CASE(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_op2_free);
    RUN_TEST_CASE(BT_OSI_FIXED_QUEUE_TEST, fixed_queue_new_op3_free);
}
