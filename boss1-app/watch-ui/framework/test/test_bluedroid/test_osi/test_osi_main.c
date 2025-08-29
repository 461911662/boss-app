/**
 * @file test_osi_code.c
 * @brief 这个文件用于测试bluedroid的osi代码
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_osi_common.h"

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/
/**
 * @details 运行所有的测试
*/
static void RunAllTests(void)
{
#ifdef CONFIG_BT_TEST_OSI_MEMORY_ENABLED
    RUN_TEST_GROUP(BT_OSI_MEM_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_ALLOCATOR_ENABLED
    RUN_TEST_GROUP(BT_OSI_ALLOCATOR_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_BUFFER_ENABLED
    RUN_TEST_GROUP(BT_OSI_BUFFER_TEST); 
#endif

#ifdef CONFIG_BT_TEST_OSI_LIST_ENABLED
    RUN_TEST_GROUP(BT_OSI_LIST_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_CONFIG_ENABLED
    RUN_TEST_GROUP(BT_OSI_CONFIG_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_SEM_ENABLED
    RUN_TEST_GROUP(BT_OSI_SEM_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_FUTURE_ENABLED
    RUN_TEST_GROUP(BT_OSI_FUTURE_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_THREAD_ENABLED
    RUN_TEST_GROUP(BT_OSI_THREAD_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_EVENT_BITS_GROUPS_ENABLED
    RUN_TEST_GROUP(BT_OSI_EventBitGroup_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_MUTEX_ENABLED
    RUN_TEST_GROUP(BT_OSI_MUTEX_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_HASH_MAP_ENABLED
    RUN_TEST_GROUP(BT_OSI_HASH_MAP_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_FIXED_QUEUE_ENABLED
    RUN_TEST_GROUP(BT_OSI_FIXED_QUEUE_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_PKT_QUEUE_ENABLED
    RUN_TEST_GROUP(BT_OSI_PKT_QUEUE_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_FIXED_PKT_QUEUE_ENABLED
    RUN_TEST_GROUP(BT_OSI_FIXED_PKT_QUEUE_TEST);
#endif

#ifdef CONFIG_BT_TEST_OSI_ALARM_ENABLED
    RUN_TEST_GROUP(BT_OSI_ALARM_TEST);
#endif

}

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
/**
 * @details 测试osi代码公共函数
*/
void test_osi_code(void)
{
    const char *argv[] = {"test_osi_code"};

    UnityMain(1, argv, RunAllTests);
}
