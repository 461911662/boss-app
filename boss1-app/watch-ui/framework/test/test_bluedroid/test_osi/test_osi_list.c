/**
 * @file test_osi_list.c
 * @brief 这个文件包含osi的list测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/allocator.h"
#include "osi/list.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_LIST_TEST);
static void os_free(void *p) {
    free(p);
}

struct custom_data {
    int num;
};

static bool handle_cb(void *data, void *context) {
    struct custom_data *pdata = (struct custom_data *)data;
    if ((pdata->num == 10) || (pdata->num == 1)) {
        return 0;
    } else {
        return 1;
    }
}

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_LIST_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_LIST_TEST)
{
}

TEST(BT_OSI_LIST_TEST, osi_list_new_free)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_clear_free)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    osi_list_clear(list);
    osi_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_clear_empty_free)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    osi_list_clear(list);
    TEST_ASSERT_TRUE(osi_list_is_empty(list));
    osi_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_length_free)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    osi_list_clear(list);
    TEST_ASSERT_EQUAL(0, osi_list_length(list));
    osi_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_prepend_free)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_prepend(list, data);
    TEST_ASSERT_EQUAL(1, osi_list_length(list));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_append_free)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);
    TEST_ASSERT_EQUAL(1, osi_list_length(list));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_append_insert_free)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);
    TEST_ASSERT_EQUAL(1, osi_list_length(list));

    char *data1 = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data1);
    osi_list_insert_after(list, osi_list_back_node(list), (void *)data1);
    TEST_ASSERT_EQUAL(2, osi_list_length(list));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_contains_free1)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    char *data = "NULL";

    TEST_ASSERT_FALSE(osi_list_contains(list, (void*)data));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_contains_free2)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);

    TEST_ASSERT_TRUE(osi_list_contains(list, (void*)data));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_front_back_free2)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);

    TEST_ASSERT_EQUAL_PTR(data, osi_list_front(list));
    TEST_ASSERT_EQUAL_PTR(data, osi_list_back(list));
    TEST_ASSERT_EQUAL_PTR(osi_list_front(list), osi_list_back(list));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_begin_back_node_free)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    TEST_ASSERT_NULL(osi_list_begin(list));
    TEST_ASSERT_NULL(osi_list_end(list));

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);
    TEST_ASSERT_EQUAL_PTR(osi_list_begin(list), osi_list_back_node(list));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_remove)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);

    TEST_ASSERT_TRUE(osi_list_remove(list, data));
    TEST_ASSERT_FALSE(osi_list_remove(list, data));

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_delete)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);

    TEST_ASSERT_TRUE(osi_list_delete(list, data));
    TEST_ASSERT_FALSE(osi_list_delete(list, data));

    os_free(data);

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_delete_node)
{
    list_t *list = osi_list_new(NULL);
    TEST_ASSERT_NOT_NULL(list);

    char *data = osi_malloc(10);
    TEST_ASSERT_NOT_NULL(data);

    osi_list_append(list, data);

    list_node_t *node = osi_list_get_node(list, data);
    TEST_ASSERT_NOT_NULL(node);

    TEST_ASSERT_NULL(osi_list_next(node));
    TEST_ASSERT_EQUAL_PTR(data, osi_list_node(node));

    TEST_ASSERT_TRUE(osi_list_delete(list, data));
    TEST_ASSERT_FALSE(osi_list_delete(list, data));

    os_free(data);

    osi_list_free(list);
}

TEST(BT_OSI_LIST_TEST, osi_list_new_foreach_free)
{
    list_t *list = osi_list_new(os_free);
    TEST_ASSERT_NOT_NULL(list);

    struct custom_data *data1 = osi_calloc(sizeof(struct custom_data));
    TEST_ASSERT_NOT_NULL(data1);
    data1->num = 1;
    osi_list_append(list, data1);

    struct custom_data *data10 = osi_calloc(sizeof(struct custom_data));
    TEST_ASSERT_NOT_NULL(data10);
    data10->num = 10;
    osi_list_append(list, data10);

    TEST_ASSERT_EQUAL_INT(2, osi_list_length(list));

    list_node_t *ret = osi_list_foreach(list, handle_cb, NULL);

    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL_PTR(data1, osi_list_node(ret));

    osi_list_free(list);
}

TEST_GROUP_RUNNER(BT_OSI_LIST_TEST)
{
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_clear_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_clear_empty_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_length_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_prepend_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_append_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_append_insert_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_contains_free1);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_contains_free2);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_front_back_free2);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_begin_back_node_free);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_remove);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_delete);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_delete_node);
  RUN_TEST_CASE(BT_OSI_LIST_TEST, osi_list_new_foreach_free);
}
