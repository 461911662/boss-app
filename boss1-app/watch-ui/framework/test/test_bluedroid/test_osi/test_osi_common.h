/**
 * @file test_osi_common.h
 * @brief 这个用来汇总osi当前所有的测试单元
*/

#ifndef _TEST_OSI_COMMON_H_
#define _TEST_OSI_COMMON_H_

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <nuttx/config.h>
#include "../../common/unity_fixture.h"

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
#ifdef CONFIG_BT_TEST_OSI_MUTEX_ENABLED
/**
 * osi mutex代码测试用例
 * @return 0表示成功，其他表示失败
 */
int test_osi_mutex_main(void);
#else
#define test_osi_mutex_main()
#endif

#ifdef CONFIG_BT_TEST_OSI_MEMORY_ENABLED
/**
 * osi memory代码测试用例
 * @return 0表示成功，其他表示失败
 */
int test_osi_memory_main(void);
#else
#define test_osi_memory_main()
#endif

#endif /*_TEST_OSI_COMMON_H_*/