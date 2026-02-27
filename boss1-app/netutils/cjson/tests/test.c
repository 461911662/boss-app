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

#include "test.h"
#include <stdio.h>

int g_test_passed = 0;
int g_test_failed = 0;

void test_print_json(cJSON *item)
{
    char *json_str = cJSON_Print(item);
    if (json_str)
    {
        printf("%s\n", json_str);
        cJSON_free(json_str);
    }
}

char *test_read_file(const char *filename)
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }

    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    content = (char*)malloc((size_t)length + 1);
    if (content == NULL)
    {
        goto cleanup;
    }

    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';

cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

void test_reset(cJSON *item)
{
    if ((item != NULL) && (item->child != NULL))
    {
        cJSON_Delete(item->child);
    }
    if ((item->valuestring != NULL) && !(item->type & cJSON_IsReference))
    {
        free(item->valuestring);
    }
    if ((item->string != NULL) && !(item->type & cJSON_StringIsConst))
    {
        free(item->string);
    }

    memset(item, 0, sizeof(cJSON));
}

void print_test_summary(void)
{
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Passed: %d\n", g_test_passed);
    printf("  Failed: %d\n", g_test_failed);
    printf("  Total:  %d\n", g_test_passed + g_test_failed);
    printf("========================================\n");

    if (g_test_failed > 0)
    {
        printf("\n*** SOME TESTS FAILED ***\n");
        exit(1);
    }
    else
    {
        printf("\n*** ALL TESTS PASSED ***\n");
        exit(0);
    }
}

/* Test suite declarations */
void suite_parse_tests(void);
void suite_print_tests(void);
void suite_misc_tests(void);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("========================================\n");
    printf("       cJSON Test Suite\n");
    printf("========================================\n");

    suite_parse_tests();
    suite_print_tests();
    suite_misc_tests();

    print_test_summary();
    return 0;
}
