/**
 * @file hci_vendor_hal.h
 * @brief 平台vendor蓝牙设备的抽象
 * @attention 可以自由学习
 */

#ifndef _HCI_VENDOR_HAL_H_
#define _HCI_VENDOR_HAL_H_

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include "esp_err.h"
#include "api/esp_bluedroid_hci.h"


/****************************************************************************
 * DEFINES
 ****************************************************************************/

#define USE_ESP_HCI_HAL 1

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @brief 获取HAL层与HCI层交互的操作句柄，具体实现在bt\host\bluedroid\hci\vendor目录
 */
extern esp_err_t
esp_bluedroid_get_hci_driver_operations(esp_bluedroid_hci_driver_operations_t *operations);

#if USE_ESP_HCI_HAL
extern void esp_bluedroid_init_hal(void);
extern void esp_bluedroid_deinit_hal(void);
#endif

/**
 * 用来安装不同厂商的hci hal，具体实现在bt\host\bluedroid\hci\vendor目录
 */
inline void setup_hci_hal(void)
{
#ifdef USE_ESP_HCI_HAL
    esp_bluedroid_init_hal();
#endif
}

/**
 * 用来拆除不同厂商的hci hal，具体实现在bt\host\bluedroid\hci\vendor目录
 */
inline void teardown_hci_hal(void)
{
#ifdef USE_ESP_HCI_HAL
    esp_bluedroid_deinit_hal();
#endif
}

/**
 * 用来注册HCI层的回调到厂商的HAL中，其中厂商的HAL实现在bt\host\bluedroid\hci\vendor目录
 */
inline esp_err_t esp_hci_host_register_callback(const esp_bluedroid_hci_driver_callbacks_t *callback)
{
    esp_err_t ret = ESP_FAIL;
    esp_bluedroid_hci_driver_operations_t p_ops;

    // 初始化HAL层
    setup_hci_hal();

    // 注册HAL层的回调
    ret = esp_bluedroid_get_hci_driver_operations(&p_ops);
    if (ret == ESP_FAIL) {
        return ret;
    }
    (void)esp_bluedroid_detach_hci_driver();
    (void)esp_bluedroid_attach_hci_driver(&p_ops);

    // 注册HCI层的回调
    if (p_ops.register_host_callback) {
        ret = p_ops.register_host_callback(callback);
    }

    return ret;
}


/**
 * 用来注销HCI层的回调到厂商的HAL中，其中厂商的HAL实现在bt\host\bluedroid\hci\vendor目录
 */
inline void esp_hci_host_unregister_callback(void)
{
    //注销HAL层的操作句柄
    (void)esp_bluedroid_detach_hci_driver();

    // 解初始化HAL层
    teardown_hci_hal();
}

#endif
