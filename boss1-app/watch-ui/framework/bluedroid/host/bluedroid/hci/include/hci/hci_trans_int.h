/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __HCI_TRANS_INT_H__
#define __HCI_TRANS_INT_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
/* HCI driver callbacks */
typedef struct bluedroid_hci_driver_callbacks {
    /**
     * @brief callback used to notify that the host can send packet to controller
     *        该回调通知host可以发送数据包到controller了。
     */
    void (*notify_host_send_available)(void);

    /**
     * @brief callback used to notify that the controller has a packet to send to the host
     *        该回调用于通知host，controller有数据包发送了。
     *
     * @param[in] data  pointer to data buffer
     * @param[in] len   length of data
     *
     * @return 0 received successfully, failed otherwise
     */
    int (*notify_host_recv)(uint8_t *data, uint16_t len);
} bluedroid_hci_driver_callbacks_t;

/* HCI driver operations */
typedef struct bluedroid_hci_driver_operations {
    /**
     * @brief send data from host to controller
     *
     * @param[in] data  pointer to data buffer
     * @param[in] len   length of data
     */
    void (*send)(uint8_t *data, uint16_t len);

    /**
     * @brief host checks whether it can send data to controller
     *
     * @return true if host can send data, false otherwise
     */
    bool (*check_send_available)(void);

    /**
     * @brief register host callback
     *
     * @param[in] callback  HCI driver callbacks
     */
    int (* register_host_callback)(const bluedroid_hci_driver_callbacks_t *callback);
} bluedroid_hci_driver_operations_t;

/**
 * @brief host checks whether it can send data to controller
 *
 * @return true if host can send data, false otherwise
 */
bool hci_host_check_send_available(void);

/**
 * @brief host sends packet to controller
 *
 * @param[in] data pointer to data buffer
 * @param[in] len  length of data in byte
 */
void hci_host_send_packet(uint8_t *data, uint16_t len);

/**
 * @brief register the HCI function interface
 *
 * @param[in] callback HCI function interface
 *
 * @return 0 register successfully, FAIL otherwise
 */
int hci_host_register_callback(const bluedroid_hci_driver_callbacks_t *callback);

/**
 * @brief 注销HCI的回调函数
 */
void hci_host_unregister_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __HCI_TRANS_INT_H__ */
