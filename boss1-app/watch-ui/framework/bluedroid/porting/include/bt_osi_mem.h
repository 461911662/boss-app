/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stddef.h>

/*
 * @brief os级别的内存申请函数
 * @param size: 申请内存大小
 * @return 申请到的内存地址
 *
 * @note 申请到的内存必须使用osi_free释放
 */
void *osi_malloc(size_t size);

/* 
 * @brief os级别的内存申请函数
 * @param n: 申请内存个数
 * @param size: 申请内存大小
 * @return 申请到的内存地址
 *
 * @note 申请到的内存必须使用osi_free释放
 * @note 申请到的内存地址是连续的
*/
void *osi_calloc(size_t n, size_t size);

/*
 * @brief os级别的内存释放函数
 * @param ptr: 释放的内存地址
 * @return 无
 * 
 * @note 释放的内存必须是由osi_malloc或者osi_calloc申请
 */
void osi_free(void *ptr);
