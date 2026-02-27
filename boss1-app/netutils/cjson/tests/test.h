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

#ifndef CJSON_TEST_H
#define CJSON_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include cJSON header */
#include "../cJSON/cJSON.h"

/* Test result counters */
extern int g_test_passed;
extern int g_test_failed;

/* Test macros */
#define TEST_ASSERT_TRUE(expr) \
    do { \
        if (expr) { \
            g_test_passed++; \
            printf("[PASS] %s\n", #expr); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_FALSE(expr) TEST_ASSERT_TRUE(!(expr))

#define TEST_ASSERT_NULL(expr) \
    do { \
        if ((expr) == NULL) { \
            g_test_passed++; \
            printf("[PASS] %s == NULL\n", #expr); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s != NULL (%s:%d)\n", #expr, __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(expr) \
    do { \
        if ((expr) != NULL) { \
            g_test_passed++; \
            printf("[PASS] %s != NULL\n", #expr); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s == NULL (%s:%d)\n", #expr, __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            g_test_passed++; \
            printf("[PASS] %s == %d\n", #actual, (int)(actual)); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s == %d, expected %d (%s:%d)\n", \
                   #actual, (int)(actual), (int)(expected), __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_DOUBLE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            g_test_passed++; \
            printf("[PASS] %s == %f\n", #actual, (double)(actual)); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s == %f, expected %f (%s:%d)\n", \
                   #actual, (double)(actual), (double)(expected), __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            g_test_passed++; \
            printf("[PASS] %s == \"%s\"\n", #actual, (actual)); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s == \"%s\", expected \"%s\" (%s:%d)\n", \
                   #actual, (actual), (expected), __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_BITS(mask, expected, actual) \
    do { \
        if (((expected) & (mask)) == ((actual) & (mask))) { \
            g_test_passed++; \
            printf("[PASS] %s & 0x%x == 0x%x\n", #actual, (int)(mask), (int)((actual) & (mask))); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s & 0x%x == 0x%x, expected 0x%x (%s:%d)\n", \
                   #actual, (int)(mask), (int)((actual) & (mask)), \
                   (int)((expected) & (mask)), __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_TRUE_MESSAGE(expr, msg) \
    do { \
        if (expr) { \
            g_test_passed++; \
            printf("[PASS] %s\n", msg); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s (%s:%d)\n", msg, __FILE__, __LINE__); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_POINTER(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            g_test_passed++; \
            printf("[PASS] %s == %p\n", #actual, (void*)(actual)); \
        } else { \
            g_test_failed++; \
            printf("[FAIL] %s == %p, expected %p (%s:%d)\n", \
                   #actual, (void*)(actual), (void*)(expected), __FILE__, __LINE__); \
        } \
    } while(0)

/* Test suite macros */
#define TEST_SUITE(name) \
    void suite_##name(void)

#define RUN_TESTSuite(name) \
    do { \
        printf("\n=== Running %s ===\n", #name); \
        suite_##name(); \
    } while(0)

#define RUN_ALL_TESTS() \
    do { \
        g_test_passed = 0; \
        g_test_failed = 0; \
        printf("\n========================================\n"); \
        printf("       cJSON Test Suite\n"); \
        printf("========================================\n");

/* Helper functions - declared here, implemented in test.c */
void print_test_summary(void);

#endif /* CJSON_TEST_H */
