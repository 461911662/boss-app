/**
 * @file esp_hci_hal_h4.c
 * @brief esp蓝牙H4的上层应用程序
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include <fcntl.h>
#include <assert.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <arch/chip/bt_ioctl.h>
#include "common/bt_trace.h"
#include "hci/hci_trans_int.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/

#define ESP_HAL_HCI_FD_INVALID (-1)
#define HCI_RECV_PACKET_SIZE (2048)
#define ESP_HAL_HCI_NAME "/dev/ttyHCI0"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/

static int hci_fd = ESP_HAL_HCI_FD_INVALID;
static pthread_t receiver_thread;
static bluedroid_hci_driver_callbacks_t callbacks;

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/

/**
 * @details 用来通知上层数据包收到了或者控制器已经做好接收数据的准备了
 */
static int register_host_callback(const bluedroid_hci_driver_callbacks_t *callback)
{
    callbacks.notify_host_recv = callback->notify_host_recv;
    callbacks.notify_host_send_available = callback->notify_host_send_available;
    HCI_TRACE_DEBUG("%s: register_host_callback", __func__);
    return ESP_OK;
}

/**
 * @details 用来上层向底层发送蓝牙数据包
 */
static void write_data(uint8_t *data, uint16_t len)
{
    assert(hci_fd != ESP_HAL_HCI_FD_INVALID);
    write(hci_fd, data, len);
}

/**
 * @details 用来检查底层蓝牙控制器是否发送有效
 */
static bool check_send_available(void)
{
    bool ret = FALSE;
    int status = 0;

    assert(hci_fd != ESP_HAL_HCI_FD_INVALID);
    if (ioctl(hci_fd, BIOC_GETSENDOK, &status) == -1) {
        HCI_TRACE_ERROR("%s check_send_available ioctl failed", __func__);
    }

    if (status) {
        ret = TRUE;
    }

    return ret;
}

/**
 * @details 蓝牙读取线程
 */
static void* hci_read_thread(void *arg)
{
    struct pollfd pfd;
    pfd.fd = hci_fd;
    pfd.events = POLLIN; // 监控可读事件
    uint8_t buffer[HCI_RECV_PACKET_SIZE];

    pthread_setname_np(pthread_self(), "esp_hal_hci_read");
    assert(hci_fd != ESP_HAL_HCI_FD_INVALID);
    while (1) {
        int ret = poll(&pfd, 1, -1); // 阻塞等待事件
        if (ret < 0) {
            HCI_TRACE_ERROR("hci_read_thread Poll error ret(%x)", ret);
            break;
        }
        if (pfd.revents & POLLIN) {
            ssize_t bytes_read = read(hci_fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                HCI_TRACE_ERROR("hci_read_thread Read error or device closed");
                break;
            }
            printf("%s: bytes_read:%d\n", __func__, bytes_read);
            // 解析HCI事件并触发回调
            if (callbacks.notify_host_recv) {
                callbacks.notify_host_recv(buffer, bytes_read);
            }

            // 通知host发送有效
            if (callbacks.notify_host_send_available) {
                callbacks.notify_host_send_available();
            }
        } else if (pfd.revents & (POLLERR | POLLHUP)) {
            HCI_TRACE_ERROR("hci_read_thread Device error or disconnected");
            break;
        }
    }
    callbacks.notify_host_recv = NULL;
    callbacks.notify_host_send_available = NULL;
    close(hci_fd);
    hci_fd = ESP_HAL_HCI_FD_INVALID;
    return NULL;
}

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
/**
 * @details esp的hal初始化函数
 */
int esp_bluedroid_init_hal(bluedroid_hci_driver_operations_t *ops)
{
    // 设置线程属性
    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if (ret != 0) {
        HCI_TRACE_ERROR("pthread_attr_init failed!");
        return -1;
    }

    ret = pthread_attr_setstacksize(&attr, 3072);
    if (ret != 0) {
        (void)pthread_attr_destroy(&attr);
        HCI_TRACE_ERROR("pthread_attr_setstacksize failed!");
        return -1;
    }

    struct sched_param param;
    ret = pthread_attr_getschedparam(&attr, &param);
    if (ret != 0) {
        (void)pthread_attr_destroy(&attr);
        HCI_TRACE_ERROR("pthread_attr_getschedparam failed!");
        return -1;
    }
    param.sched_priority = 100;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret != 0) {
        (void)pthread_attr_destroy(&attr);
        HCI_TRACE_ERROR("pthread_attr_setschedparam failed!");
        return -1;
    }

    // 打开设备
    hci_fd = open(ESP_HAL_HCI_NAME, O_RDWR);
    if (hci_fd < 0) {
        HCI_TRACE_ERROR("%s open %s failed!", __func__, ESP_HAL_HCI_NAME);
        return -1;
    }

    // 初始化HCI operations
    ops->send = write_data;
    ops->check_send_available = check_send_available;
    ops->register_host_callback = register_host_callback;

    // 启动读线程
    if (pthread_create(&receiver_thread, &attr, hci_read_thread, NULL) != 0) {
        HCI_TRACE_ERROR("Create receiver thread failed!");
        close(hci_fd);
        hci_fd = ESP_HAL_HCI_FD_INVALID;
        return -1;
    }

    if (ioctl(hci_fd, BIOC_POWERON, NULL) == -1) {
        HCI_TRACE_ERROR("%s power on ioctl failed", __func__);
    }

    struct btparam_s bt_param;
    if (ioctl(hci_fd, BIOC_GETSENDOK, &bt_param) == -1) {
        HCI_TRACE_ERROR("%s check_send_available ioctl failed", __func__);
        return -1;
    }
    HCI_TRACE_DEBUG("bt send status:%d\n", bt_param.resp.is_host_send);

    HCI_TRACE_DEBUG("ESP Bluedroid HAL initialized successfully");
}

/**
 * @details esp的hal注销函数
 */
void esp_bluedroid_deinit_hal(void)
{
    if (hci_fd != ESP_HAL_HCI_FD_INVALID) {
        if (ioctl(hci_fd, BIOC_POWEROFF, NULL) == -1) {
            HCI_TRACE_ERROR("%s check_send_available ioctl failed", __func__);
        }

        if (close(hci_fd) != 0) {
            HCI_TRACE_ERROR("%s close %s failed!", __func__, ESP_HAL_HCI_NAME);
        }
    } else {
        HCI_TRACE_WARNING("hci_fd is invalid!");
    }

    pthread_cancel(receiver_thread);

    HCI_TRACE_DEBUG("ESP Bluedroid HAL deinitialized successfully");
}
