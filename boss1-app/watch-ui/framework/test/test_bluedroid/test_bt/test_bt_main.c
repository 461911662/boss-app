/**
 * @file test_bt_code.c
 * @brief 这个文件用于测试bluedroid的bt代码
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_bt_common.h"

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/
/**
 * @details 运行所有的测试
*/
static void RunAllTests(void)
{
    RUN_TEST_GROUP(BT_ESP_HAL_INIT_TEST);
}

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
/**
 * @details 测试osi代码公共函数
*/
void test_bt_code(void)
{
    const char *argv[] = {"test_bt_code"};

    UnityMain(1, argv, RunAllTests);
}