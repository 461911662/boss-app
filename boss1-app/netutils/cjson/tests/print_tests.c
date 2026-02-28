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
 * Print Number Tests
 ****************************************************************************/

static void test_print_number_int(void)
{
    cJSON *item = cJSON_CreateNumber(123);
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "123"));
    
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_number_float(void)
{
    cJSON *item = cJSON_CreateNumber(123.45);
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_number_negative(void)
{
    cJSON *item = cJSON_CreateNumber(-456.78);
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "-456"));
    
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_number_precision(void)
{
    cJSON *item = cJSON_CreateNumber(3.14159265358979);
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(item);
}

/****************************************************************************
 * Print String Tests
 ****************************************************************************/

static void test_print_string_simple(void)
{
    cJSON *item = cJSON_CreateString("hello");
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "\"hello\""));
    
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_string_escape(void)
{
    cJSON *item = cJSON_CreateString("hello\nworld");
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_string_empty(void)
{
    cJSON *item = cJSON_CreateString("");
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("\"\"", str);
    
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_string_unicode(void)
{
    cJSON *item = cJSON_CreateString("中文");
    TEST_ASSERT_NOT_NULL(item);
    
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(item);
}

/****************************************************************************
 * Print Array Tests
 ****************************************************************************/

static void test_print_array_empty(void)
{
    cJSON *array = cJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(array);
    
    char *str = cJSON_Print(array);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("[]", str);
    
    cJSON_free(str);
    cJSON_Delete(array);
}

static void test_print_array_numbers(void)
{
    cJSON *array = cJSON_CreateArray();
    TEST_ASSERT_NOT_NULL(array);
    
    cJSON_AddItemToArray(array, cJSON_CreateNumber(1));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(2));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(3));
    
    char *str = cJSON_Print(array);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "1"));
    TEST_ASSERT_NOT_NULL(strstr(str, "2"));
    TEST_ASSERT_NOT_NULL(strstr(str, "3"));
    
    cJSON_free(str);
    cJSON_Delete(array);
}

static void test_print_array_mixed(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateNumber(1));
    cJSON_AddItemToArray(array, cJSON_CreateString("test"));
    cJSON_AddItemToArray(array, cJSON_CreateTrue());
    
    char *str = cJSON_Print(array);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(array);
}

static void test_print_array_nested(void)
{
    cJSON *array = cJSON_CreateArray();
    cJSON *inner1 = cJSON_CreateArray();
    cJSON_AddItemToArray(inner1, cJSON_CreateNumber(1));
    cJSON_AddItemToArray(inner1, cJSON_CreateNumber(2));
    cJSON_AddItemToArray(array, inner1);
    
    char *str = cJSON_Print(array);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(array);
}

/****************************************************************************
 * Print Object Tests
 ****************************************************************************/

static void test_print_object_empty(void)
{
    cJSON *obj = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(obj);
    
    char *str = cJSON_Print(obj);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "{"));
    TEST_ASSERT_NOT_NULL(strstr(str, "}"));
    
    cJSON_free(str);
    cJSON_Delete(obj);
}

static void test_print_object_single(void)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "name", "value");
    
    char *str = cJSON_Print(obj);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "name"));
    TEST_ASSERT_NOT_NULL(strstr(str, "value"));
    
    cJSON_free(str);
    cJSON_Delete(obj);
}

static void test_print_object_multiple(void)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "a", "1");
    cJSON_AddNumberToObject(obj, "b", 2);
    cJSON_AddTrueToObject(obj, "c");
    
    char *str = cJSON_Print(obj);
    TEST_ASSERT_NOT_NULL(str);
    
    cJSON_free(str);
    cJSON_Delete(obj);
}

static void test_print_object_nested(void)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON *inner = cJSON_CreateObject();
    cJSON_AddStringToObject(inner, "x", "y");
    cJSON_AddItemToObject(obj, "inner", inner);
    
    char *str = cJSON_Print(obj);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(strstr(str, "inner"));
    
    cJSON_free(str);
    cJSON_Delete(obj);
}

/****************************************************************************
 * Print Value Tests
 ****************************************************************************/

static void test_print_value_null(void)
{
    cJSON *item = cJSON_CreateNull();
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("null", str);
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_value_true(void)
{
    cJSON *item = cJSON_CreateTrue();
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("true", str);
    cJSON_free(str);
    cJSON_Delete(item);
}

static void test_print_value_false(void)
{
    cJSON *item = cJSON_CreateFalse();
    char *str = cJSON_Print(item);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("false", str);
    cJSON_free(str);
    cJSON_Delete(item);
}

/****************************************************************************
 * Minify Tests
 ****************************************************************************/

static void test_minify_simple(void)
{
    char json[] = "{\n  \"name\"  :  \n\"test\"\n}";
    cJSON_Minify(json);
    TEST_ASSERT_EQUAL_STRING("{\"name\":\"test\"}", json);
}

static void test_minify_array(void)
{
    char json[] = "[ 1 , 2 , 3 ]";
    cJSON_Minify(json);
    TEST_ASSERT_EQUAL_STRING("[1,2,3]", json);
}

static void test_minify_whitespace(void)
{
    char json[] = "  {  \"key\"  :  \"value\"  }  ";
    cJSON_Minify(json);
    TEST_ASSERT_EQUAL_STRING("{\"key\":\"value\"}", json);
}

static void test_minify_nested(void)
{
    char json[] = "{ \"a\" : [ 1 , 2 ] , \"b\" : { \"c\" : 3 } }";
    cJSON_Minify(json);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"a\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"b\""));
}

static void test_minify_empty(void)
{
    char json[] = "   ";
    cJSON_Minify(json);
    TEST_ASSERT_EQUAL_STRING("", json);
}

/****************************************************************************
 * Test Suite
 ****************************************************************************/

TEST_SUITE(print_tests)
{
    printf("\n=== Print Number Tests ===\n");
    test_print_number_int();
    test_print_number_float();
    test_print_number_negative();
    test_print_number_precision();

    printf("\n=== Print String Tests ===\n");
    test_print_string_simple();
    test_print_string_escape();
    test_print_string_empty();
    test_print_string_unicode();

    printf("\n=== Print Array Tests ===\n");
    test_print_array_empty();
    test_print_array_numbers();
    test_print_array_mixed();
    test_print_array_nested();

    printf("\n=== Print Object Tests ===\n");
    test_print_object_empty();
    test_print_object_single();
    test_print_object_multiple();
    test_print_object_nested();

    printf("\n=== Print Value Tests ===\n");
    test_print_value_null();
    test_print_value_true();
    test_print_value_false();

    printf("\n=== Minify Tests ===\n");
    test_minify_simple();
    test_minify_array();
    test_minify_whitespace();
    test_minify_nested();
    test_minify_empty();
}
