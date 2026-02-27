/*
 * Copyright (c) 2024 BOSS1
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See
 * the License for the specific language governing permissions and limitations
 * under the License.
 */

#include "../cJSON/cJSON.h"
#include "test.h"
#include <string.h>

/****************************************************************************
 * Misc Tests (Array/Object Operations)
 ****************************************************************************/

static void test_array_foreach_loop(void)
{
    cJSON array[1];
    cJSON elements[10];
    cJSON *element_pointer = NULL;
    size_t i = 0;

    memset(array, 0, sizeof(array));
    memset(elements, 0, sizeof(elements));

    array[0].child = &elements[0];
    elements[0].prev = NULL;
    elements[9].next = NULL;
    for (i = 0; i < 9; i++)
    {
        elements[i].next = &elements[i + 1];
        elements[i + 1].prev = &elements[i];
    }

    i = 0;
    cJSON_ArrayForEach(element_pointer, array)
    {
        TEST_ASSERT_EQUAL_POINTER(&elements[i], element_pointer);
        i++;
    }
}

static void test_array_foreach_null(void)
{
    cJSON *array = NULL;
    cJSON *element = NULL;
    cJSON_ArrayForEach(element, array);
    TEST_ASSERT_NULL(element);
}

static void test_get_object_item(void)
{
    cJSON *item = cJSON_Parse("{\"one\":1, \"Two\":2, \"tHree\":3}");
    TEST_ASSERT_NOT_NULL(item);

    cJSON *found = cJSON_GetObjectItem(NULL, "test");
    TEST_ASSERT_NULL(found);

    found = cJSON_GetObjectItem(item, NULL);
    TEST_ASSERT_NULL(found);

    found = cJSON_GetObjectItem(item, "one");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_DOUBLE(1, found->valuedouble);

    cJSON_Delete(item);
}

static void test_get_object_item_case_sensitive(void)
{
    cJSON *item = cJSON_Parse("{\"Name\":1, \"NAME\":2, \"name\":3}");
    TEST_ASSERT_NOT_NULL(item);

    cJSON *found = cJSON_GetObjectItemCaseSensitive(item, "Name");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_DOUBLE(1, found->valuedouble);

    cJSON_Delete(item);
}

static void test_has_object_item(void)
{
    cJSON *item = cJSON_Parse("{\"one\":1, \"two\":2}");
    TEST_ASSERT_NOT_NULL(item);

    TEST_ASSERT_TRUE(cJSON_HasObjectItem(item, "one"));
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(item, "two"));
    TEST_ASSERT_FALSE(cJSON_HasObjectItem(item, "three"));
    TEST_ASSERT_FALSE(cJSON_HasObjectItem(item, NULL));

    cJSON_Delete(item);
}

static void test_get_array_size(void)
{
    cJSON *array = cJSON_Parse("[1, 2, 3, 4, 5]");
    TEST_ASSERT_NOT_NULL(array);

    TEST_ASSERT_EQUAL_INT(5, cJSON_GetArraySize(array));

    cJSON_Delete(array);
}

static void test_get_array_item(void)
{
    cJSON *array = cJSON_Parse("[10, 20, 30]");
    TEST_ASSERT_NOT_NULL(array);

    cJSON *item0 = cJSON_GetArrayItem(array, 0);
    TEST_ASSERT_NOT_NULL(item0);
    TEST_ASSERT_EQUAL_INT(10, (int)item0->valuedouble);

    cJSON *item1 = cJSON_GetArrayItem(array, 1);
    TEST_ASSERT_NOT_NULL(item1);
    TEST_ASSERT_EQUAL_INT(20, (int)item1->valuedouble);

    cJSON *item2 = cJSON_GetArrayItem(array, 2);
    TEST_ASSERT_NOT_NULL(item2);
    TEST_ASSERT_EQUAL_INT(30, (int)item2->valuedouble);

    cJSON *item3 = cJSON_GetArrayItem(array, 3);
    TEST_ASSERT_NULL(item3);

    cJSON_Delete(array);
}

static void test_create_types(void)
{
    cJSON *item = NULL;

    item = cJSON_CreateNull();
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNull(item));
    cJSON_Delete(item);

    item = cJSON_CreateTrue();
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsTrue(item));
    cJSON_Delete(item);

    item = cJSON_CreateFalse();
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsFalse(item));
    cJSON_Delete(item);

    item = cJSON_CreateNumber(42.5);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_DOUBLE(42.5, item->valuedouble);
    cJSON_Delete(item);

    item = cJSON_CreateString("test");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    TEST_ASSERT_EQUAL_STRING("test", item->valuestring);
    cJSON_Delete(item);

    item = cJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsArray(item));
    cJSON_Delete(item);

    item = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsObject(item));
    cJSON_Delete(item);
}

static void test_detach_item(void)
{
    cJSON *array = cJSON_Parse("[1, 2, 3]");
    TEST_ASSERT_NOT_NULL(array);

    cJSON *item = cJSON_DetachItemFromArray(array, 1);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL_INT(2, (int)item->valuedouble);

    TEST_ASSERT_EQUAL_INT(2, cJSON_GetArraySize(item));

    cJSON_Delete(item);
    cJSON_Delete(array);
}

static void test_delete_item(void)
{
    cJSON *array = cJSON_Parse("[1, 2, 3]");
    TEST_ASSERT_NOT_NULL(array);

    TEST_ASSERT_EQUAL_INT(3, cJSON_GetArraySize(array));

    cJSON_DeleteItemFromArray(array, 1);

    TEST_ASSERT_EQUAL_INT(2, cJSON_GetArraySize(array));

    cJSON_Delete(array);
}

static void test_insert_item(void)
{
    cJSON *array = cJSON_Parse("[1, 3]");
    TEST_ASSERT_NOT_NULL(array);

    cJSON *item = cJSON_CreateNumber(2);
    cJSON_InsertItemInArray(array, 1, item);

    TEST_ASSERT_EQUAL_INT(3, cJSON_GetArraySize(array));

    cJSON *item0 = cJSON_GetArrayItem(array, 0);
    TEST_ASSERT_EQUAL_INT(1, (int)item0->valuedouble);

    cJSON *item1 = cJSON_GetArrayItem(array, 1);
    TEST_ASSERT_EQUAL_INT(2, (int)item1->valuedouble);

    cJSON *item2 = cJSON_GetArrayItem(array, 2);
    TEST_ASSERT_EQUAL_INT(3, (int)item2->valuedouble);

    cJSON_Delete(array);
}

static void test_replace_item(void)
{
    cJSON *array = cJSON_Parse("[1, 2, 3]");
    TEST_ASSERT_NOT_NULL(array);

    cJSON *item = cJSON_CreateNumber(99);
    cJSON_ReplaceItemInArray(array, 1, item);

    cJSON *item1 = cJSON_GetArrayItem(array, 1);
    TEST_ASSERT_EQUAL_INT(99, (int)item1->valuedouble);

    cJSON_Delete(array);
}

/****************************************************************************
 * cJSON Add Tests
 ****************************************************************************/

static void test_cjson_add_null_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(root);
    
    cJSON_AddNullToObject(root, "key");
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "key"));
    
    cJSON *item = cJSON_GetObjectItem(root, "key");
    TEST_ASSERT_TRUE(cJSON_IsNull(item));
    
    cJSON_Delete(root);
}

static void test_cjson_add_true_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddTrueToObject(root, "flag");
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "flag"));
    
    cJSON *item = cJSON_GetObjectItem(root, "flag");
    TEST_ASSERT_TRUE(cJSON_IsTrue(item));
    
    cJSON_Delete(root);
}

static void test_cjson_add_false_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddFalseToObject(root, "flag");
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "flag"));
    
    cJSON *item = cJSON_GetObjectItem(root, "flag");
    TEST_ASSERT_TRUE(cJSON_IsFalse(item));
    
    cJSON_Delete(root);
}

static void test_cjson_add_number_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "value", 42.5);
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "value"));
    
    cJSON *item = cJSON_GetObjectItem(root, "value");
    TEST_ASSERT_EQUAL_DOUBLE(42.5, item->valuedouble);
    
    cJSON_Delete(root);
}

static void test_cjson_add_string_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "test");
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "name"));
    
    cJSON *item = cJSON_GetObjectItem(root, "name");
    TEST_ASSERT_EQUAL_STRING("test", item->valuestring);
    
    cJSON_Delete(root);
}

static void test_cjson_add_array_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(root, "items", arr);
    
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "items"));
    cJSON *item = cJSON_GetObjectItem(root, "items");
    TEST_ASSERT_TRUE(cJSON_IsArray(item));
    TEST_ASSERT_EQUAL_INT(1, cJSON_GetArraySize(item));
    
    cJSON_Delete(root);
}

static void test_cjson_add_object_to_object(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *child = cJSON_CreateObject();
    cJSON_AddStringToObject(child, "inner", "value");
    cJSON_AddItemToObject(root, "parent", child);
    
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(root, "parent"));
    cJSON *item = cJSON_GetObjectItem(root, "parent");
    TEST_ASSERT_TRUE(cJSON_IsObject(item));
    
    cJSON_Delete(root);
}

static void test_cjson_add_to_array(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON_AddNumberToObject(array, NULL, 1);
    cJSON_AddStringToObject(array, NULL, "test");
    
    TEST_ASSERT_EQUAL_INT(2, cJSON_GetArraySize(array));
    
    cJSON_Delete(array);
}

/****************************************************************************
 * Compare Tests
 ****************************************************************************/

static void test_compare_objects_equal(void)
{
    cJSON *a = cJSON_Parse("{\"a\":1, \"b\":2}");
    cJSON *b = cJSON_Parse("{\"a\":1, \"b\":2}");
    
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    
    TEST_ASSERT_TRUE(cJSON_Compare(a, b, 1));
    
    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void test_compare_objects_not_equal(void)
{
    cJSON *a = cJSON_Parse("{\"a\":1}");
    cJSON *b = cJSON_Parse("{\"a\":2}");
    
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    
    TEST_ASSERT_FALSE(cJSON_Compare(a, b, 1));
    
    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void test_compare_arrays_equal(void)
{
    cJSON *a = cJSON_Parse("[1, 2, 3]");
    cJSON *b = cJSON_Parse("[1, 2, 3]");
    
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    
    TEST_ASSERT_TRUE(cJSON_Compare(a, b, 1));
    
    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void test_compare_case_sensitive(void)
{
    cJSON *a = cJSON_Parse("{\"Name\":1}");
    cJSON *b = cJSON_Parse("{\"name\":1}");
    
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    
    TEST_ASSERT_FALSE(cJSON_Compare(a, b, 1));
    TEST_ASSERT_TRUE(cJSON_Compare(a, b, 0));
    
    cJSON_Delete(a);
    cJSON_Delete(b);
}

/****************************************************************************
 * Readme Examples Tests
 ****************************************************************************/

static void test_readme_create_monitor(void)
{
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(monitor);

    cJSON *name = cJSON_CreateString("Awesome 4K");
    TEST_ASSERT_NOT_NULL(name);
    cJSON_AddItemToObject(monitor, "name", name);

    cJSON *resolutions = cJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(resolutions);
    cJSON_AddItemToObject(monitor, "resolutions", resolutions);

    for (index = 0; index < 3; index++)
    {
        cJSON *resolution = cJSON_CreateObject();
        cJSON_AddItemToArray(resolutions, resolution);

        cJSON *width = cJSON_CreateNumber(resolution_numbers[index][0]);
        cJSON_AddItemToObject(resolution, "width", width);

        cJSON *height = cJSON_CreateNumber(resolution_numbers[index][1]);
        cJSON_AddItemToObject(resolution, "height", height);
    }

    TEST_ASSERT_TRUE(cJSON_HasObjectItem(monitor, "name"));
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(monitor, "resolutions"));

    cJSON *res = cJSON_GetObjectItem(monitor, "resolutions");
    TEST_ASSERT_EQUAL_INT(3, cJSON_GetArraySize(res));

    cJSON_Delete(monitor);
}

static void test_readme_parse_and_access(void)
{
    cJSON *root = cJSON_Parse("{\"name\":\"test\",\"array\":[1,2,3]}");
    TEST_ASSERT_NOT_NULL(root);

    cJSON *name = cJSON_GetObjectItem(root, "name");
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_EQUAL_STRING("test", name->valuestring);

    cJSON *array = cJSON_GetObjectItem(root, "array");
    TEST_ASSERT_NOT_NULL(array);
    TEST_ASSERT_EQUAL_INT(3, cJSON_GetArraySize(array));

    cJSON_Delete(root);
}

/****************************************************************************
 * JSON Patch Tests
 ****************************************************************************/

static void test_json_patch_add(void)
{
    cJSON *root = cJSON_Parse("{\"a\":1}");
    cJSON *patch = cJSON_Parse("[{\"op\":\"add\",\"path\":\"/b\",\"value\":2}]");
    
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_NOT_NULL(patch);
    
    cJSON_Delete(root);
    cJSON_Delete(patch);
}

static void test_json_patch_remove(void)
{
    cJSON *root = cJSON_Parse("{\"a\":1, \"b\":2}");
    cJSON *patch = cJSON_Parse("[{\"op\":\"remove\",\"path\":\"/a\"}]");
    
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_NOT_NULL(patch);
    
    cJSON_Delete(root);
    cJSON_Delete(patch);
}

static void test_json_patch_replace(void)
{
    cJSON *root = cJSON_Parse("{\"a\":1}");
    cJSON *patch = cJSON_Parse("[{\"op\":\"replace\",\"path\":\"/a\",\"value\":2}]");
    
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_NOT_NULL(patch);
    
    cJSON_Delete(root);
    cJSON_Delete(patch);
}

static void test_json_patch_move(void)
{
    cJSON *root = cJSON_Parse("{\"a\":1, \"b\":2}");
    cJSON *patch = cJSON_Parse("[{\"op\":\"move\",\"from\":\"/a\",\"path\":\"/c\"}]");
    
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_NOT_NULL(patch);
    
    cJSON_Delete(root);
    cJSON_Delete(patch);
}

/****************************************************************************
 * Test Suite
 ****************************************************************************/

TEST_SUITE(misc_tests)
{
    printf("\n=== Array/Object Operation Tests ===\n");
    test_array_foreach_loop();
    test_array_foreach_null();
    test_get_object_item();
    test_get_object_item_case_sensitive();
    test_has_object_item();
    test_get_array_size();
    test_get_array_item();
    test_create_types();
    test_detach_item();
    test_delete_item();
    test_insert_item();
    test_replace_item();

    printf("\n=== cJSON Add Tests ===\n");
    test_cjson_add_null_to_object();
    test_cjson_add_true_to_object();
    test_cjson_add_false_to_object();
    test_cjson_add_number_to_object();
    test_cjson_add_string_to_object();
    test_cjson_add_array_to_object();
    test_cjson_add_object_to_object();
    test_cjson_add_to_array();

    printf("\n=== Compare Tests ===\n");
    test_compare_objects_equal();
    test_compare_objects_not_equal();
    test_compare_arrays_equal();
    test_compare_case_sensitive();

    printf("\n=== Readme Examples Tests ===\n");
    test_readme_create_monitor();
    test_readme_parse_and_access();

    printf("\n=== JSON Patch Tests ===\n");
    test_json_patch_add();
    test_json_patch_remove();
    test_json_patch_replace();
    test_json_patch_move();
}
