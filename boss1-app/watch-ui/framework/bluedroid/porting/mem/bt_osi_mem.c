/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stddef.h>
#include <stdlib.h>

/*
 * @details 使用系统调用malloc申请一块指定大小的普通内存
 */
void *osi_malloc(size_t size)
{
    return malloc(size);
}

/*
 * @details 使用系统调用calloc申请多块块指定大小的内存，并初始化为0
 */
void *osi_calloc(size_t n, size_t size)
{
    return calloc(n, size);
}

/*
 * @details 使用系统调用free释放原来申请的内存
 */
void osi_free(void *ptr)
{
    free(ptr);
}
