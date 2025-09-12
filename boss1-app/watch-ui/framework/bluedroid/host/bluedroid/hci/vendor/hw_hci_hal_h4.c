/**
 * @file hw_hci_hal_h4.c
 * @brief 硬件层面的hal_h4，用于对接不同的芯片
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <string.h>
#include "common/bt_target.h"
#include "common/bt_trace.h"
#include "hci/hci_trans_int.h"
#include "hci_log/bt_hci_log.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define USE_ESP32_HCI_H4 1

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static bluedroid_hci_driver_operations_t s_hci_driver_ops = { 0 }; // 需要加锁


/**
 * 用来安装不同厂商的hci hal，具体实现在bt\host\bluedroid\hci\vendor目录
 */
static int setup_hci_hal(bluedroid_hci_driver_operations_t *ops)
{
#if USE_ESP32_HCI_H4 == 1
    extern int esp_bluedroid_init_hal(bluedroid_hci_driver_operations_t *ops);
    return esp_bluedroid_init_hal(ops);
#endif
}

/**
 * 用来拆除不同厂商的hci hal，具体实现在bt\host\bluedroid\hci\vendor目录
 */
static void teardown_hci_hal(void)
{
#if USE_ESP32_HCI_H4 == 1
    void esp_bluedroid_deinit_hal(void);
    esp_bluedroid_deinit_hal();
#endif
}

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
/**
 * @details host检查controller是否可以发送数据包
 */
bool hci_host_check_send_available(void)
{
    bool can_send = false;
    if (s_hci_driver_ops.check_send_available) {
        can_send = s_hci_driver_ops.check_send_available();
        HCI_TRACE_DEBUG("%s check_send_available %s send\n", __func__, can_send ? "can" : "can't");
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
        HCI_TRACE_DEBUG("%s packet sending, len=%d", __func__, len);
        s_hci_driver_ops.send(data, len);
    }
}

/**
 * @details hci host注册与底层通信回调, 该回调主要包含以下几个功能：
 * 1. 通知host可以发送数据包了。
 * 2. 通知host，controller已经发送了一个数据包，等待host接收。
 */
int hci_host_register_callback(const bluedroid_hci_driver_callbacks_t *callback)
{
    int ret = 0;

    if (!callback) {
        HCI_TRACE_ERROR("%s invalid function parameter", __func__);
        return ESP_FAIL;
    }

    // 初始化HAL层
    ret = setup_hci_hal(&s_hci_driver_ops);
    if (ret) {
        HCI_TRACE_ERROR("%s setup_hci_hal failed", __func__);
        return ret;
    }

    if (s_hci_driver_ops.register_host_callback) {
        ret = s_hci_driver_ops.register_host_callback(callback);
    } else {
        HCI_TRACE_ERROR("%s register_host_callback is NULL", __func__);
        ret = -1;
    }
    return ret;
}

/**
 * @details hci host通知控制器，注销与底层通信回调
 */
void hci_host_unregister_callback(void)
{
    s_hci_driver_ops.send                   = NULL;
    s_hci_driver_ops.check_send_available   = NULL;
    s_hci_driver_ops.register_host_callback = NULL;

    teardown_hci_hal();
}
