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
#include <math.h>

/****************************************************************************
 * Parse Number Tests
 ****************************************************************************/

static void test_parse_number_int(void)
{
    cJSON *item = cJSON_Parse("123");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_INT(123, (int)item->valuedouble);
    TEST_ASSERT_EQUAL_INT(123, item->valueint);
    cJSON_Delete(item);
}

static void test_parse_number_negative(void)
{
    cJSON *item = cJSON_Parse("-456");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_INT(-456, (int)item->valuedouble);
    cJSON_Delete(item);
}

static void test_parse_number_float(void)
{
    cJSON *item = cJSON_Parse("123.45");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_DOUBLE(123.45, item->valuedouble);
    cJSON_Delete(item);
}

static void test_parse_number_scientific(void)
{
    cJSON *item = cJSON_Parse("1.23e10");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_DOUBLE(1.23e10, item->valuedouble);
    cJSON_Delete(item);

    item = cJSON_Parse("1.23E-5");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_DOUBLE(1.23e-5, item->valuedouble);
    cJSON_Delete(item);
}

static void test_parse_number_zero(void)
{
    cJSON *item = cJSON_Parse("0");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    TEST_ASSERT_EQUAL_INT(0, (int)item->valuedouble);
    cJSON_Delete(item);
}

static void test_parse_number_leading_zero(void)
{
    cJSON *item = cJSON_Parse("01");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    cJSON_Delete(item);
}

/****************************************************************************
 * Parse String Tests
 ****************************************************************************/

static void test_parse_string_simple(void)
{
    cJSON *item = cJSON_Parse("\"hello\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    TEST_ASSERT_EQUAL_STRING("hello", item->valuestring);
    cJSON_Delete(item);
}

static void test_parse_string_escape(void)
{
    cJSON *item = cJSON_Parse("\"hello\\nworld\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    cJSON_Delete(item);

    item = cJSON_Parse("\"tab\\tchar\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    cJSON_Delete(item);
}

static void test_parse_string_unicode(void)
{
    cJSON *item = cJSON_Parse("\"\\u0041\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    cJSON_Delete(item);
}

static void test_parse_string_empty(void)
{
    cJSON *item = cJSON_Parse("\"\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    TEST_ASSERT_EQUAL_STRING("", item->valuestring);
    cJSON_Delete(item);
}

/****************************************************************************
 * Parse Array Tests
 ****************************************************************************/

static void test_parse_array_empty(void)
{
    cJSON *item = cJSON_Parse("[]");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsArray(item));
    TEST_ASSERT_EQUAL_INT(0, cJSON_GetArraySize(item));
    cJSON_Delete(item);
}

static void test_parse_array_numbers(void)
{
    cJSON *item = cJSON_Parse("[1, 2, 3]");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsArray(item));
    TEST_ASSERT_EQUAL_INT(3, cJSON_GetArraySize(item));
    
    cJSON *elem = cJSON_GetArrayItem(item, 0);
    TEST_ASSERT_NOT_NULL(elem);
    TEST_ASSERT_EQUAL_INT(1, (int)elem->valuedouble);
    
    elem = cJSON_GetArrayItem(item, 1);
    TEST_ASSERT_NOT_NULL(elem);
    TEST_ASSERT_EQUAL_INT(2, (int)elem->valuedouble);
    
    elem = cJSON_GetArrayItem(item, 2);
    TEST_ASSERT_NOT_NULL(elem);
    TEST_ASSERT_EQUAL_INT(3, (int)elem->valuedouble);
    
    cJSON_Delete(item);
}

static void test_parse_array_mixed(void)
{
    cJSON *item = cJSON_Parse("[1, \"hello\", true, null, {}]");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsArray(item));
    TEST_ASSERT_EQUAL_INT(5, cJSON_GetArraySize(item));
    cJSON_Delete(item);
}

static void test_parse_array_nested(void)
{
    cJSON *item = cJSON_Parse("[[1, 2], [3, 4]]");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsArray(item));
    TEST_ASSERT_EQUAL_INT(2, cJSON_GetArraySize(item));
    
    cJSON *sub1 = cJSON_GetArrayItem(item, 0);
    TEST_ASSERT_NOT_NULL(sub1);
    TEST_ASSERT_TRUE(cJSON_IsArray(sub1));
    
    cJSON_Delete(item);
}

/****************************************************************************
 * Parse Object Tests
 ****************************************************************************/

static void test_parse_object_empty(void)
{
    cJSON *item = cJSON_Parse("{}");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsObject(item));
    TEST_ASSERT_EQUAL_INT(0, cJSON_GetArraySize(item));
    cJSON_Delete(item);
}

static void test_parse_object_single(void)
{
    cJSON *item = cJSON_Parse("{\"key\": \"value\"}");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsObject(item));
    
    cJSON *val = cJSON_GetObjectItem(item, "key");
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_STRING("value", val->valuestring);
    
    cJSON_Delete(item);
}

static void test_parse_object_multiple(void)
{
    cJSON *item = cJSON_Parse("{\"a\": 1, \"b\": 2, \"c\": 3}");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsObject(item));
    
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(item, "a"));
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(item, "b"));
    TEST_ASSERT_TRUE(cJSON_HasObjectItem(item, "c"));
    
    cJSON_Delete(item);
}

static void test_parse_object_nested(void)
{
    cJSON *item = cJSON_Parse("{\"outer\": {\"inner\": 1}}");
    TEST_ASSERT_NOT_NULL(item);
    
    cJSON *outer = cJSON_GetObjectItem(item, "outer");
    TEST_ASSERT_NOT_NULL(outer);
    TEST_ASSERT_TRUE(cJSON_IsObject(outer));
    
    cJSON *inner = cJSON_GetObjectItem(outer, "inner");
    TEST_ASSERT_NOT_NULL(inner);
    TEST_ASSERT_EQUAL_INT(1, (int)inner->valuedouble);
    
    cJSON_Delete(item);
}

/****************************************************************************
 * Parse Value Tests
 ****************************************************************************/

static void test_parse_value_null(void)
{
    cJSON *item = cJSON_Parse("null");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNull(item));
    cJSON_Delete(item);
}

static void test_parse_value_true(void)
{
    cJSON *item = cJSON_Parse("true");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsTrue(item));
    TEST_ASSERT_FALSE(cJSON_IsFalse(item));
    cJSON_Delete(item);
}

static void test_parse_value_false(void)
{
    cJSON *item = cJSON_Parse("false");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsFalse(item));
    TEST_ASSERT_FALSE(cJSON_IsTrue(item));
    cJSON_Delete(item);
}

/****************************************************************************
 * Parse Hex4 Tests
 ****************************************************************************/

static void test_parse_hex4_simple(void)
{
    cJSON *item = cJSON_Parse("\"\\u0041\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    cJSON_Delete(item);
}

static void test_parse_hex4_chinese(void)
{
    cJSON *item = cJSON_Parse("\"\\u4e2d\\u6587\"");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsString(item));
    cJSON_Delete(item);
}

/****************************************************************************
 * Parse With Options Tests
 ****************************************************************************/

static void test_parse_with_opts_comments(void)
{
    cJSON *item = cJSON_Parse("//comment\n123");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    cJSON_Delete(item);

    item = cJSON_Parse("/*comment*/456");
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_TRUE(cJSON_IsNumber(item));
    cJSON_Delete(item);
}

static void test_parse_invalid(void)
{
    cJSON *item = cJSON_Parse("{invalid}");
    TEST_ASSERT_NULL(item);
    
    item = cJSON_Parse("[1, 2,");
    TEST_ASSERT_NULL(item);
}

/****************************************************************************
 * Test Suite
 ****************************************************************************/

TEST_SUITE(parse_tests)
{
    printf("\n=== Parse Number Tests ===\n");
    test_parse_number_int();
    test_parse_number_negative();
    test_parse_number_float();
    test_parse_number_scientific();
    test_parse_number_zero();
    test_parse_number_leading_zero();

    printf("\n=== Parse String Tests ===\n");
    test_parse_string_simple();
    test_parse_string_escape();
    test_parse_string_unicode();
    test_parse_string_empty();

    printf("\n=== Parse Array Tests ===\n");
    test_parse_array_empty();
    test_parse_array_numbers();
    test_parse_array_mixed();
    test_parse_array_nested();

    printf("\n=== Parse Object Tests ===\n");
    test_parse_object_empty();
    test_parse_object_single();
    test_parse_object_multiple();
    test_parse_object_nested();

    printf("\n=== Parse Value Tests ===\n");
    test_parse_value_null();
    test_parse_value_true();
    test_parse_value_false();

    printf("\n=== Parse Hex4 Tests ===\n");
    test_parse_hex4_simple();
    test_parse_hex4_chinese();

    printf("\n=== Parse With Options Tests ===\n");
    test_parse_with_opts_comments();
    test_parse_invalid();
}
