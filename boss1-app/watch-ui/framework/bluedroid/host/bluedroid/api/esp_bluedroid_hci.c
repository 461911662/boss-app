/**
 * @file esp_bluedroid_hci.c
 * @brief 修改components\bt\host\bluedroid\api\esp_bluedroid_hci.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <string.h>
#include "esp_log.h"
#include "esp_bluedroid_hci.h"
#include "common/bt_target.h"
#include "hci/hci_trans_int.h"
#include "hci/hci_vendor_hal.h"
#include "hci_log/bt_hci_log.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define LOG_TAG "HCI_API"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static esp_bluedroid_hci_driver_operations_t s_hci_driver_ops = { 0 };

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details HOST连接HCI的操作回调，主要包括以下方法：
 * 1、连接HCI向控制器发送函数
 * 2、连接HCI检查控制器发送ready的函数
 * 3、连接HCI向HAL注册回调的函数
 */
esp_err_t esp_bluedroid_attach_hci_driver(esp_bluedroid_hci_driver_operations_t *p_ops)
{
    if (!p_ops) {
        ESP_LOGE(LOG_TAG, "%s invalid function parameter", __func__);
        return ESP_FAIL;
    }

    s_hci_driver_ops.send                   = p_ops->send;
    s_hci_driver_ops.check_send_available   = p_ops->check_send_available;
    s_hci_driver_ops.register_host_callback = p_ops->register_host_callback;

    return ESP_OK;
}

/**
 * @details HOST断开HCI的操作回调，主要包括以下方法：
 * 1、断开HCI向控制器发送函数
 * 2、断开HCI检查控制器发送ready的函数
 * 3、断开HCI向HAL注册回调的函数
 */
esp_err_t esp_bluedroid_detach_hci_driver(void)
{
    s_hci_driver_ops.send                   = NULL;
    s_hci_driver_ops.check_send_available   = NULL;
    s_hci_driver_ops.register_host_callback = NULL;

    return ESP_OK;
}

/**
 * @details host检查controller是否可以发送数据包
 */
bool hci_host_check_send_available(void)
{
    bool can_send = false;
    if (s_hci_driver_ops.check_send_available) {
        can_send = s_hci_driver_ops.check_send_available();
    }
    return can_send;
}

/**
 * @details host发送数据包的API
 */
void hci_host_send_packet(uint8_t *data, uint16_t len)
{
#if (BT_HCI_LOG_INCLUDED == TRUE)
    bt_hci_log_record_hci_data(data[0], &data[1], len - 1);
#endif
    if (s_hci_driver_ops.send) {
        s_hci_driver_ops.send(data, len);
    }
}

/**
 * @details hci host注册与底层通信回调, 该回调主要包含以下几个功能：
 * 1. 通知host可以发送数据包了。
 * 2. 通知host，controller已经发送了一个数据包，等待host接收。
 */
esp_err_t hci_host_register_callback(const esp_bluedroid_hci_driver_callbacks_t *callback)
{
    esp_err_t ret = ESP_FAIL;

    if (!callback) {
        ESP_LOGE(LOG_TAG, "%s invalid function parameter", __func__);
        return ESP_FAIL;
    }

    ret = esp_hci_host_register_callback(callback);
    return ret;
}

/**
 * @details hci host通知控制器，注销与底层通信回调
 */
void hci_host_unregister_callback(void)
{
    esp_hci_host_unregister_callback();
}
