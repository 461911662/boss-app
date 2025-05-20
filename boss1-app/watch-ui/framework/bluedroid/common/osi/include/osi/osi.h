/**
 * @file osi.h
 * @brief 移植components\bt\common\osi\include\osi.h
 * @attention 可以自由学习
 */

#ifndef _OSI_H_
#define _OSI_H_

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define UNUSED_ATTR __attribute__((unused))

#define CONCAT(a, b) a##b
#define COMPILE_ASSERT(x)

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
/**
 * osi接口初始化函数
 * @return 0表示成功，其他表示失败
 */
int osi_init(void);

/**
 * osi接口解初始化函数
 * @return 0表示成功，其他表示失败
 */
void osi_deinit(void);

#endif /*_OSI_H_*/
