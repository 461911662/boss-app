/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi_manager/wifi_manager.c
 *
 * WiFi Manager Implementation - Single Signal with siginfo_t
 ******t**********************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <signal.h>
#include <nuttx/spinlock.h>
#include <nuttx/irq.h>
#include <nuttx/queue.h>
#include <uv.h>
#include <nuttx/net/ioctl.h>
#include <wireless/wapi.h>

#include "wifi_manager.h"
#include "wifi/wifi.h"
#include "pipeline/pipeline.h"
#include "watchui/log.h"

/* Single WiFi signal for all events */
#define WIFI_SIGNAL (SIGRTMIN + 10)

/****************************************************************************
 * Logging Macros
 ****************************************************************************/
#define wfinfo(fmt, ...)    appinfo("[WifiManager] " fmt, ##__VA_ARGS__)
#define wfdbg(fmt, ...)    appdbg("[WifiManager] " fmt, ##__VA_ARGS__)
#define wfwarn(fmt, ...)   appwarn("[WifiManager] " fmt, ##__VA_ARGS__)
#define wferr(fmt, ...)    apperr("[WifiManager] " fmt, ##__VA_ARGS__)

static wifi_manager_t g_manager = {0};

/* Event List Struct */
struct wifi_event_list {
    FAR const char *ifname;
    FAR sq_entry_t *head;
};

/* Event mapping table */
struct wifi_event {
    FAR sq_entry_t entry;
    int wifi_event_id;      /* WIFI_EVENT -> WIFI_ADPT_EVT */
};

static struct wifi_event_list g_sta_evt_list = {NULL, NULL};
static struct wifi_event_list g_softap_evt_list = {NULL, NULL};

/* Forward declarations */
static void pipeline_continue_cb(uv_async_t *handle);

/* Global async handle for pipeline continuation */
static uv_async_t g_pipeline_async;

/* 带自旋锁保护的事件链表 - 适用于信号处理和线程上下文
 * 生产者：信号处理函数（中断上下文）
 * 消费者：libuv 回调（线程上下文）
 */
struct recv_event {
    sq_entry_t entry;
    int wifi_event_id;
};

static sq_queue_t g_recv_event_list = {NULL, NULL};  /* 接收事件链表 */
static spinlock_t g_recv_event_lock = SP_UNLOCKED;   /* NuttX 自旋锁 */

/* 将事件添加到接收链表 - 在信号处理中调用
 * 使用自旋锁保护，信号安全
 */
static void add_wifi_event_to_list(int wifi_event_id)
{
    struct recv_event *evt = (struct recv_event *)malloc(sizeof(struct recv_event));
    if (!evt) {
        wfwarn("add_wifi_event_to_list: malloc failed!\n");
        return;
    }
    
    evt->wifi_event_id = wifi_event_id;
    
    irqstate_t flags = spin_lock_irqsave(&g_recv_event_lock);
    sq_addlast(&evt->entry, &g_recv_event_list);
    spin_unlock_irqrestore(&g_recv_event_lock, flags);
}

/* 从接收链表取出事件 - 在libuv回调中调用
 * 使用自旋锁保护
 */
static bool get_wifi_event_from_list(int *wifi_event_id)
{
    irqstate_t flags = spin_lock_irqsave(&g_recv_event_lock);
    struct recv_event *evt = (struct recv_event *)sq_remfirst(&g_recv_event_list);
    spin_unlock_irqrestore(&g_recv_event_lock, flags);
    
    if (evt) {
        *wifi_event_id = evt->wifi_event_id;
        free(evt);
        return true;
    }
    return false;
}

void add_event_to_list(FAR sq_entry_t **head, int event_id)
{
    struct wifi_event *event = (struct wifi_event *)malloc(sizeof(struct wifi_event));
    event->wifi_event_id = event_id;

    if ((*head) == NULL) {
        (*head) = (sq_entry_t *)event;
        event->entry.flink = NULL;
    } else {
        event->entry.flink = sq_next((*head));
        sq_next((*head)) = (sq_entry_t *)event;
    }
}

static void wifi_collect_events(void)
{
    g_sta_evt_list.ifname = WIFI_STA_DEVNAME;
    add_event_to_list(&g_sta_evt_list.head, WIFI_EVENT_SCAN_DONE);
    add_event_to_list(&g_sta_evt_list.head, WIFI_EVENT_STA_START);
    add_event_to_list(&g_sta_evt_list.head, WIFI_EVENT_STA_CONNECTED);
    add_event_to_list(&g_sta_evt_list.head, WIFI_EVENT_STA_DISCONNECTED);
    add_event_to_list(&g_sta_evt_list.head, WIFI_EVENT_STA_AUTHMODE_CHANGE);
    add_event_to_list(&g_sta_evt_list.head, WIFI_EVENT_STA_STOP);

    g_softap_evt_list.ifname = WIFI_AP_DEVNAME;
    add_event_to_list(&g_softap_evt_list.head, WIFI_EVENT_AP_START);
    add_event_to_list(&g_softap_evt_list.head, WIFI_EVENT_AP_STOP);
    add_event_to_list(&g_softap_evt_list.head, WIFI_EVENT_AP_STACONNECTED);
    add_event_to_list(&g_softap_evt_list.head, WIFI_EVENT_AP_STADISCONNECTED);
}

static void wifi_decollect_events(void)
{
    sq_entry_t *entry = g_sta_evt_list.head;
    while (entry != NULL) {
        sq_entry_t *next = sq_next(entry);
        free(entry);
        entry = next;
    }
    g_sta_evt_list.ifname = NULL;
    g_sta_evt_list.head = NULL;

    entry = g_softap_evt_list.head;
    while (entry != NULL) {
        sq_entry_t *next = sq_next(entry);
        free(entry);
        entry = next;
    }
    g_softap_evt_list.ifname = NULL;
    g_softap_evt_list.head = NULL;
}

/* Signal handler - runs in signal context, must be fast and safe
 *
 * 只负责将事件添加到接收链表，然后通知 libuv 处理
 * 不直接操作 pipeline，避免并发问题
 */
static void wifi_sigaction_handler(int sig, siginfo_t *info, void *context)
{
    (void)sig;
    (void)context;

    /* Get WiFi event ID from sival_int (直接使用原始 ESP-IDF 事件 ID) */
    int wifi_event_id = info->si_value.sival_int;

    wfdbg("Received WiFi event %d\n", wifi_event_id);

    /* 添加到接收链表 */
    add_wifi_event_to_list(wifi_event_id);

    /* 通知 libuv 有事件待处理 */
    uv_async_send(&g_pipeline_async);
}

/* Pipeline continuation callback - runs in libuv context
 *
 * 从接收链表取出所有事件，使用 pipeline_handle_event 处理
 */
static void pipeline_continue_cb(uv_async_t *handle)
{
    (void)handle;

    int wifi_event_id;

    /* 处理链表中的所有事件 */
    while (get_wifi_event_from_list(&wifi_event_id)) {
        /* 使用 pipeline_handle_event 处理事件
         * 该函数会遍历所有 active pipelines，找到等待该事件的 node
         * 并执行相应的处理
         */
        wfdbg("Processed WiFi event %d\n", wifi_event_id);
        pipeline_handle_event(wifi_event_id, NULL);
    }
}

static void subscribe_wifi_events(struct wifi_event_list *evt_list)
{
    /* Subscribe ALL ESP-IDF WiFi events */
    int ret;
    struct ifreq ifr;

    /* Create socket for ioctl */
    int sock = wapi_make_socket();
    if (sock < 0) {
        wferr("Failed to create socket for event subscription: %d\n", errno);
        return;
    }

    memcpy(ifr.ifr_name, evt_list->ifname, IFNAMSIZ);
    sq_entry_t *next = NULL;
    sq_entry_t *entry = evt_list->head;
    while (entry != NULL) {
        next = sq_next(entry);

        ifr.ifr_mii_notify_pid = 0;  /* 0 = current process */
        ifr.ifr_mii_notify_event.sigev_notify = SIGEV_SIGNAL;
        ifr.ifr_mii_notify_event.sigev_signo = WIFI_SIGNAL;  /* Single signal for all */
        ifr.ifr_mii_notify_event.sigev_value.sival_int = ((struct wifi_event*)entry)->wifi_event_id;

        ret = ioctl(sock, SIOCMIINOTIFY, (unsigned long)&ifr);
        if (ret < 0) {
            wfwarn("Failed to subscribe to WiFi event %d: %d\n",
                ((struct wifi_event*)entry)->wifi_event_id, errno);
        } else {
            wfdbg("Subscribed to WiFi event %d\n",
                ((struct wifi_event*)entry)->wifi_event_id);
        }

        entry = next;
    }

    close(sock);
}

static void desubscribe_wifi_events(struct wifi_event_list *evt_list)
{
    /* Subscribe ALL ESP-IDF WiFi events */
    int ret;
    struct ifreq ifr;

    /* Create socket for ioctl */
    int sock = wapi_make_socket();
    if (sock < 0) {
        wferr("Failed to create socket for event subscription: %d\n", errno);
        return;
    }

    memcpy(ifr.ifr_name, evt_list->ifname, IFNAMSIZ);
    sq_entry_t *next = NULL;
    sq_entry_t *entry = evt_list->head;
    while (entry != NULL) {
        next = sq_next(entry);

        ifr.ifr_mii_notify_event.sigev_notify = SIGEV_NONE;
        ifr.ifr_mii_notify_event.sigev_signo = WIFI_SIGNAL;  /* Single signal for all */
        ifr.ifr_mii_notify_event.sigev_value.sival_int = ((struct wifi_event*)entry)->wifi_event_id;

        ret = ioctl(sock, SIOCMIINOTIFY, (unsigned long)&ifr);
        if (ret < 0) {
            wfwarn("Failed to desubscribe to WiFi event %d: %d\n",
                ((struct wifi_event*)entry)->wifi_event_id, errno);
        } else {
            wfdbg("DeSubscribed to WiFi event %d\n",
                ((struct wifi_event*)entry)->wifi_event_id);
        }

        entry = next;
    }

    close(sock);
}

int wifi_manager_init(uv_loop_t *loop)
{
    if (g_manager.loop != NULL) {
        return 0;  // Already initialized
    }

    g_manager.loop = loop;

    /* Initialize async handle for pipeline continuation */
    uv_async_init(loop, &g_pipeline_async, pipeline_continue_cb);

    /* Register signal handler using sigaction (not uv_signal_t) 
     * This allows us to access siginfo_t */
    struct sigaction sa;
    sa.sa_sigaction = wifi_sigaction_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(WIFI_SIGNAL, &sa, NULL) < 0) {
        wferr("Failed to register WiFi signal handler: %d\n", errno);
        return -errno;
    }

    /* collect all events */
    wifi_collect_events();

    /* 1. Subscribe sta event */
    subscribe_wifi_events(&g_sta_evt_list);

    /* 2. Subscribe softap event */
    subscribe_wifi_events(&g_softap_evt_list);

    /* Decollect all events */
    wifi_decollect_events();

    return 0;
}

wifi_manager_t *wifi_manager_get(void)
{
    return &g_manager;
}

void wifi_manager_deinit(void)
{
    /* Restore default signal handler */
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(WIFI_SIGNAL, &sa, NULL);

    /* collect all events */
    wifi_collect_events();

    /* 1. DeSubscribe sta event */
    desubscribe_wifi_events(&g_sta_evt_list);

    /* 2. DeSubscribe softap event */
    desubscribe_wifi_events(&g_softap_evt_list);

    /* Decollect all events */
    wifi_decollect_events();

    /* Destroy connections */
    if (g_manager.sta_conn) {
        wifi_connection_destroy(g_manager.sta_conn);
        g_manager.sta_conn = NULL;
    }
    if (g_manager.ap_conn) {
        wifi_connection_destroy(g_manager.ap_conn);
        g_manager.ap_conn = NULL;
    }
    
    uv_close((uv_handle_t *)&g_pipeline_async, NULL);
    g_manager.loop = NULL;
}

/* High-level API: Start AP */
int wifi_manager_start_ap(const char *ssid, const char *password,
                          in_addr_t ipaddr, in_addr_t netmask,
                          void (*on_success)(void *ctx),
                          void (*on_error)(void *ctx, int error),
                          void *user_ctx)
{
    if (g_manager.ap_conn) {
        return -EBUSY;  // AP already exists
    }

    /* Create connection object */
    wifi_connection_t *conn = wifi_connection_create(ssid, password, true);
    if (!conn) {
        return -ENOMEM;
    }

    conn->local_ip = ipaddr;
    conn->netmask = netmask;

    g_manager.ap_conn = conn;

    /* Store callbacks in connection context */
    conn->on_success = on_success;
    conn->on_error = on_error;
    conn->user_ctx = user_ctx;

    /* Create and start pipeline */
    pipeline_t *p = pipeline_ap_start_create(conn, conn->user_ctx);

    if (!p) {
        wifi_connection_destroy(conn);
        g_manager.ap_conn = NULL;
        return -ENOMEM;
    }

    conn->active_pipeline = p;
    return pipeline_start(p);
}

/* High-level API: Connect to STA */
int wifi_manager_connect(const char *ssid, const char *password,
                         void (*on_success)(void *ctx),
                         void (*on_error)(void *ctx, int error),
                         void *user_ctx)
{
    if (g_manager.sta_conn) {
        return -EBUSY;  // STA already connected
    }

    /* Create connection object - false for STA mode */
    wifi_connection_t *conn = wifi_connection_create(ssid, password, false);
    if (!conn) {
        return -ENOMEM;
    }

    g_manager.sta_conn = conn;

    /* Store callbacks in connection context */
    conn->on_success = on_success;
    conn->on_error = on_error;
    conn->user_ctx = user_ctx;

    /* Create and start pipeline */
    pipeline_t *p = pipeline_sta_connect_create(conn, NULL, conn->user_ctx);

    if (!p) {
        wifi_connection_destroy(conn);
        g_manager.sta_conn = NULL;
        return -ENOMEM;
    }

    conn->active_pipeline = p;
    return pipeline_start(p);
}

void wifi_manager_handle_event(int event_id, void *event_data)
{
    /* Dispatch event to pipelines */
    pipeline_handle_event(event_id, event_data);
}

/* Test callbacks */
static void test_ap_success_cb(void *ctx)
{
    wfdbg("AP started successfully!\n");
    (void)ctx;
}

static void test_ap_error_cb(void *ctx, int error)
{
    wfdbg("AP start failed: %d\n", error);
    (void)ctx;
}

static void test_sta_success_cb(void *ctx)
{
    wfinfo("STA connected successfully!\n");
    (void)ctx;
}

static void test_sta_error_cb(void *ctx, int error)
{
    wfdbg("STA connect failed: %d\n", error);
    (void)ctx;
}

/* Test function: Start AP mode */
void test_wifi_ap_start(void)
{
    wfinfo("\n=== Testing WiFi AP Start ===\n");

    int ret = wifi_manager_start_ap(
        "ESP32-S3-AP",           /* SSID */
        "12345678",              /* Password (8+ chars) */
        inet_addr("192.168.4.1"),  /* IP Address */
        inet_addr("255.255.255.0"), /* Netmask */
        test_ap_success_cb,      /* Success callback */
        test_ap_error_cb,        /* Error callback */
        NULL                     /* User context */
    );

    if (ret < 0) {
        wferr("Failed to start AP pipeline: %d\n", ret);
    } else {
        wfdbg("AP pipeline started, waiting for events...\n");
    }
}

/* Test function: Connect to STA */
void test_wifi_sta_connect(void)
{
    wfinfo("\n=== Testing WiFi STA Connect ===\n");

    int ret = wifi_manager_connect(
        "YourNetwork",           /* SSID */
        "YourPassword",         /* Password */
        test_sta_success_cb,    /* Success callback */
        test_sta_error_cb,      /* Error callback */
        NULL                    /* User context */
    );

    if (ret < 0) {
        wferr("Failed to start STA pipeline: %d\n", ret);
    } else {
        wfdbg("STA pipeline started, waiting for events...\n");
    }
}
