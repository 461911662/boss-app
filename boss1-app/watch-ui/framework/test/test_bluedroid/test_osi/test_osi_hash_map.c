/**
 * @file test_hash_map.c
 * @brief 这个文件包含所有的osi hash_map测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"
#include "osi/hash_functions.h"
#include "osi/hash_map.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_OSI_HASH_MAP_TEST);
static void key_free(void *key) {
    free(key);
}

static void data_free(void *data) {
    free(data);
}

static bool key_equal(const void *key1, const void *key2) {
    uint32_t len1 = strlen(key1);
    uint32_t len2 = strlen(key2);
    if (len1 != len2) {
        return false;
    }
    return strcmp(key1, key2) == 0;
}

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_OSI_HASH_MAP_TEST)
{
}

TEST_TEAR_DOWN(BT_OSI_HASH_MAP_TEST)
{
}

TEST(BT_OSI_HASH_MAP_TEST, mutex_new_free)
{
    hash_map_t *map = NULL;
    map = hash_map_new(10, hash_function_naive, NULL, NULL, NULL);
    TEST_ASSERT_NOT_NULL(map);
    hash_map_free(map);
}

TEST(BT_OSI_HASH_MAP_TEST, mutex_new_native_free)
{
    hash_map_t *map = NULL;
    map = hash_map_new(10, hash_function_naive, NULL, NULL, NULL);
    TEST_ASSERT_NOT_NULL(map);

    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)2));
    TEST_ASSERT_EQUAL(true, hash_map_has_key(map, (const void *)1));
    TEST_ASSERT_EQUAL(2, (uint32_t)hash_map_get(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_erase(map, (const void *)1));
    TEST_ASSERT_EQUAL(NULL, hash_map_get(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)2));
    hash_map_clear(map);
    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)2));
    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)10));
    TEST_ASSERT_EQUAL(10, (uint32_t)hash_map_get(map, (const void *)1));

    hash_map_free(map);
}

TEST(BT_OSI_HASH_MAP_TEST, mutex_new_int_free)
{
    hash_map_t *map = NULL;
    map = hash_map_new(10, hash_function_integer, NULL, NULL, NULL);
    TEST_ASSERT_NOT_NULL(map);

    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)2));
    TEST_ASSERT_EQUAL(true, hash_map_has_key(map, (const void *)1));
    TEST_ASSERT_EQUAL(2, (uint32_t)hash_map_get(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_erase(map, (const void *)1));
    TEST_ASSERT_EQUAL(NULL, hash_map_get(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)2));
    hash_map_clear(map);
    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, (const void *)1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)2));
    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)1, (void *)10));
    TEST_ASSERT_EQUAL(10, (uint32_t)hash_map_get(map, (const void *)1));

    hash_map_free(map);    
}

TEST(BT_OSI_HASH_MAP_TEST, mutex_new_pointer_free)
{
    hash_map_t *map = NULL;
    map = hash_map_new(10, hash_function_pointer, key_free, data_free, key_equal);
    TEST_ASSERT_NOT_NULL(map);

    char *key1 = (char *)malloc(10);
    strcpy(key1, "key1");

    int *data1 = (int *)malloc(sizeof(int));
    *data1 = 10;

    char *key2 = (char *)malloc(10);
    strcpy(key2, "key2");

    int *data2 = (int *)malloc(sizeof(int));
    *data2 = 20;

    char *key3 = (char *)malloc(10);
    strcpy(key3, "key3");

    int *data3 = (int *)malloc(sizeof(int));
    *data3 = 30;

    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, (const void *)key1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)key1, (void *)data1));
    TEST_ASSERT_EQUAL(true, hash_map_has_key(map, (const void *)key1));
    TEST_ASSERT_EQUAL(data1, (void *)hash_map_get(map, key1));

    TEST_ASSERT_EQUAL(true, hash_map_erase(map, key1));
    TEST_ASSERT_EQUAL(NULL, hash_map_get(map, "key1"));

    key1 = (char *)malloc(10);
    strcpy(key1, "key1");
    data1 = (int *)malloc(sizeof(int));
    *data1 = 10;
    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)key1, (void *)data1));
    hash_map_clear(map);
    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, (const void *)key1));

    key1 = (char *)malloc(10);
    strcpy(key1, "key1");
    data1 = (int *)malloc(sizeof(int));
    *data1 = 10;
    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)key1, (void *)data1));

    key1 = (char *)malloc(10);
    strcpy(key1, "key1");
    data1 = (int *)malloc(sizeof(int));
    *data1 = 33;
    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)key1, (void *)data1));
    TEST_ASSERT_EQUAL(data1, (void *)hash_map_get(map, key1));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)key2, (void *)data2));
    TEST_ASSERT_EQUAL(data2, (void *)hash_map_get(map, key2));
    TEST_ASSERT_EQUAL(true, hash_map_set(map, (const void *)key3, (void *)data3));
    TEST_ASSERT_EQUAL(data3, (void *)hash_map_get(map, key3));

    hash_map_free(map);    
}

TEST(BT_OSI_HASH_MAP_TEST, mutex_new_string_free)
{
    hash_map_t *map = NULL;
    map = hash_map_new(10, hash_function_string, NULL, NULL, NULL);
    TEST_ASSERT_NOT_NULL(map);

    TEST_ASSERT_EQUAL(false, hash_map_has_key(map, "key1"));

    TEST_ASSERT_EQUAL(true, hash_map_set(map, "key1", (void *)32));
    TEST_ASSERT_EQUAL(true, hash_map_has_key(map, "key1"));
    TEST_ASSERT_EQUAL(32, (uint32_t)hash_map_get(map, "key1"));

    hash_map_free(map); 
}

TEST_GROUP_RUNNER(BT_OSI_HASH_MAP_TEST)
{
    RUN_TEST_CASE(BT_OSI_HASH_MAP_TEST, mutex_new_free);
    RUN_TEST_CASE(BT_OSI_HASH_MAP_TEST, mutex_new_native_free);
    RUN_TEST_CASE(BT_OSI_HASH_MAP_TEST, mutex_new_int_free);
    RUN_TEST_CASE(BT_OSI_HASH_MAP_TEST, mutex_new_pointer_free);
    RUN_TEST_CASE(BT_OSI_HASH_MAP_TEST, mutex_new_string_free);
}
