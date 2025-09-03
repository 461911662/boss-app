/**
 * @file test_osi_alarm.c
 * @brief 这个文件包含所有的osi alarm测试用例
*/

/****************************************************************************
 * INCLUDE
 ****************************************************************************/
#include <testing/unity.h>
#include "test_bt_common.h"

extern void esp_bluedroid_init_hal(void);
extern void esp_bluedroid_deinit_hal(void);

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
TEST_GROUP(BT_ESP_HAL_INIT_TEST);

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
TEST_SETUP(BT_ESP_HAL_INIT_TEST)
{
}

TEST_TEAR_DOWN(BT_ESP_HAL_INIT_TEST)
{
}

TEST(BT_ESP_HAL_INIT_TEST, alarm_new_free)
{
    esp_bluedroid_init_hal();
    esp_bluedroid_deinit_hal();
}

TEST_GROUP_RUNNER(BT_ESP_HAL_INIT_TEST)
{
    RUN_TEST_CASE(BT_ESP_HAL_INIT_TEST, alarm_new_free);
}
