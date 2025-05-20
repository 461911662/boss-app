/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stddef.h>

void *osi_malloc(size_t size)
{
    return malloc(size);
}

void *osi_calloc(size_t n, size_t size)
{
    return calloc(n, size);
}

void osi_free(void *ptr)
{
    free(ptr);
}
