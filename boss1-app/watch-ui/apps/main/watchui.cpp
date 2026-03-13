/**
 * @file watchui/apps/main/watchui.cpp
 * @brief watchui 类实现文件
 */

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <cstdio>
#include <iostream>

#include <mooncake.h>

#include "watchui.h"
#include "watchui/log.h"
#include "watchui_c2cxx.h"
#include <csignal>

#include "include/pwm.h"

#include "config/config_manager.h"
#include "ipc/ipc_server.h"

#ifdef CONFIG_WIRELESS_WAPI
#include <arpa/inet.h>
#include <net/wifi/wifi.h>
#include <net/wifi_manager/wifi_manager.h>
#include "net/dns_server/dns_server.h"
#include "net/dhcp_server/dhcp_server.h"
#include "httpd_server.h"
#endif

static std::unique_ptr<WatchUI> _watchui_instance = nullptr;
static void *g_config_handler = nullptr;

/**
 * @brief watchui 主线程异步触发
 */
void watchui_async_send() {
    appinfo("watchui_async_send called");
    
    if (!_watchui_instance) {
        appwarn("_watchui_instance is null in watchui_async_send");
        return;
    }

#ifdef CONFIG_LIBUV
    auto handle = _watchui_instance->get_async_handle();
    if (!handle) {
        appwarn("Async handle is null");
        return;
    }
    
    if (!handle->loop) {
        appwarn("Async handle has no loop (event loop not running?)");
        return;
    }
    
    if (!uv_loop_alive(handle->loop)) {
        appwarn("Event loop is not alive");
        return;
    }

    int ret = uv_async_send(handle);
    if (ret != 0) {
        appwarn("uv_async_send failed, ret: %d", ret);
    }
#endif
}

static void setSignalHandler() {
    auto handler = [](int sig) {
        appinfo("signal[%d] received, exiting", sig);
        if (_watchui_instance) {
            _watchui_instance->_destory();
            _watchui_instance.reset();
        }
        exit(0);
    };

    signal(SIGHUP, handler);
    signal(SIGINT, handler); // ctrl+c
    signal(SIGQUIT, handler);
    signal(SIGTERM, handler); // default exit program
}

/**
 * @brief 获取 WatchUI 单例
 */
WatchUI& GetWatchUI() {
    if (!_watchui_instance) {
        _watchui_instance = std::make_unique<WatchUI>();
    }
    return *_watchui_instance;
}

/**
 * @brief watchui_main 应用入口函数
 */
int watchui_main(int argc, char *argv[]) {
    appdbg("Entry");
    setSignalHandler();
    GetWatchUI().run(); // forever
    return 0;
}

void WatchUI::setup() {
    appdbg("setup");
    int ret = 0;

    // install breath led
    get_mooncake().createExtension(std::make_unique<PwmWorker>());

    /* Initialize pipeline manager */
    ret = pipeline_manager_init(get_loop());
    if (ret < 0) {
        appwarn("pipeline_manager_init failed: %d", ret);
    }

    /* Initialize config */
    g_config_handler = config_load();
    if (!g_config_handler) {
        appwarn("config_load failed");
    }

    /* Initialize IPC server */
    ret = ipc_server_init();
    if (ret < 0) {
        appwarn("ipc_server_init failed: %d", ret);
    }

#ifdef CONFIG_WIRELESS_WAPI
    // Initialize WiFi Manager with Pipeline
    appinfo("Initializing WiFi Manager...");
    ret = wifi_manager_init(get_loop());
    if (ret < 0) {
        appwarn("wifi_manager_init failed: %d", ret);
    } else {
        appinfo("WiFi Manager initialized successfully");
    }

    set_sta_ssid("CMCC-EHd6");
    set_sta_password("ev7mkuyv");
    
    // Example: Start STA mode using Pipeline
    // Note: This is just an example. In production, you should move this to user action callback
    // test_wifi_sta_connect();

    // Example: Start AP mode using Pipeline
    // Note: This is just an example. In production, you should move this to user action callback
    test_wifi_ap_start();

    // dhcp_server_start(get_loop(), "wlan1",
    //                  _ap_ipaddr, _dhcp_start_ipaddr,
    //                  _dhcp_end_ipaddr, _ap_netmask);

    // dns_server_start(get_loop(), _ap_ipaddr);

    // httpd_server_start();
#endif
}

#ifdef CONFIG_WIRELESS_WAPI
void WatchUI::test_wifi_ap_start() {
    appinfo("Starting WiFi AP using Pipeline...");

    int ret = wifi_manager_start_ap(
        _ap_ssid,                // SSID from class member
        _ap_password,            // Password from class member
        _ap_ipaddr,              // IP Address from class member
        _ap_netmask,             // Netmask from class member
        // Success callback
        [](void *ctx) {
            // (void)ctx;
            appinfo("WiFi AP started successfully via Pipeline!");
            if (ctx) {
                WatchUI* ui = static_cast<WatchUI*>(ctx);
                dns_server_start(ui->get_loop(), ui->_ap_ipaddr);
                httpd_server_start();
            }
        },
        // Error callback
        [](void *ctx, int error) {
            (void)ctx;
            appwarn("WiFi AP failed to start: %d", error);
        },
        // nullptr                // User context
        this
    );

    if (ret < 0) {
        appwarn("Failed to create AP pipeline: %d", ret);
    }
}

void WatchUI::test_wifi_sta_connect() {
    if (_sta_ssid[0] == '\0') {
        appwarn("STA SSID is not set");
        return;
    }

    appinfo("Connecting to WiFi STA: %s", _sta_ssid);

    int ret = wifi_manager_connect(
        _sta_ssid,               // SSID from class member
        _sta_password,           // Password from class member
        // Success callback
        [](void *ctx) {
            appinfo("WiFi STA connected successfully!");
            // if (ctx) {
            //     WatchUI* ui = static_cast<WatchUI*>(ctx);
            //     ui->test_wifi_ap_start();
            // }
        },
        // Error callback
        [](void *ctx, int error) {
            (void)ctx;
            appwarn("WiFi STA failed to connect: %d", error);
        },
        this
    );

    if (ret < 0) {
        appwarn("Failed to create STA connect pipeline: %d", ret);
    }
}
#endif

void WatchUI::update() {
    ipc_server_poll();
}

void WatchUI::destroy() {
    appinfo("destroy - cleaning up extensions");

#ifdef CONFIG_WIRELESS_WAPI
    wifi_manager_deinit();
    dns_server_stop();
#endif

    ipc_server_deinit();

    // TODO: destory pipeline

    if (g_config_handler) {
        config_save(g_config_handler);
        config_free(g_config_handler);
        g_config_handler = nullptr;
    }

    get_mooncake().resetExtensionManager();
}
