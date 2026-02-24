/**
 * @file watchui/apps/include/watchui.h
 * @brief watchui 类头文件
*/

#ifndef __WATCHUI_H__
#define __WATCHUI_H__

/* Global defines ------------------------------------------------------------ */
/* Global enums -------------------------------------------------------------- */
/* Global typedefs ----------------------------------------------------------- */
/* Global variables ---------------------------------------------------------- */
/* Global variable prototypes ------------------------------------------------ */


/* Includes ------------------------------------------------------------------ */

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#ifdef CONFIG_LIBUV
#include "uv.h"
#else
#include "unistd.h"
#endif

#include "ability/ability.h"
#include <memory>
#include <netinet/in.h>

#include "watchui/log.h"

using namespace mooncake;

/* Global function prototypes ------------------------------------------------ */

class WatchUI;
/**
 * @brief 获取 WatchUI 单例
 */
WatchUI& GetWatchUI();

/**
 * @brief 销毁 WatchUI 单例
 */
void DestroyWatchUI();

class WatchUI {
public:
    WatchUI() {
        mc = std::make_unique<mooncake::Mooncake>();
        _ap_ipaddr = 0x0164a8c0;      // 192.168.100.1 in network byte order
        _ap_netmask = 0x00ffffff;     // 255.255.255.0 in network byte order
        _dhcp_start_ipaddr = 0x6464a8c0; // 192.168.100.100 in network byte order
        _dhcp_end_ipaddr = 0xff64a8c0; // 192.168.100.255 in network byte order
    }
    ~WatchUI() {
        if (mc) {
            mc.reset();
        }
    }

    Mooncake& get_mooncake() { return *mc; };

#ifdef CONFIG_LIBUV
    void set_refresh_rate(uint32_t fps) {
        if (fps > 0) {
            _refresh_rate = fps;
        }
    }

    uint32_t get_refresh_rate() { return _refresh_rate; }

    uv_loop_t* get_loop() { return loop; }
    uv_async_t* get_async_handle() { return &async_handle; }
#endif

    void setup();
    void update();
    void destroy();

    void run() {
        // 1. pre setup
    #ifdef CONFIG_LIBUV
        (void)mkdir(CONFIG_DEV_PIPE_VFS_PATH, 0777);
        loop = uv_default_loop();
        async_handle.data = this;
        int ret = uv_async_init(loop, &async_handle, async_handler_internal);
        if (ret != 0) {
            apperr("async_handle failed, ret: %d", ret);
            return;
        }
        periodic_timer.data = this;
        ret = uv_timer_init(loop, &periodic_timer);
        if (ret != 0) {
            apperr("periodic_timer failed, ret: %d", ret);
            return;
        }
    #endif

        // 2. setup
        setup();

        // 3. start loop
    #ifdef CONFIG_LIBUV
        uint32_t period_ms = 1000 / _refresh_rate;
        uv_timer_start(&periodic_timer, periodic_handler_internal, 0, period_ms);
        uv_run(loop, UV_RUN_DEFAULT);
    #else
        while(true) {
            update();
            mc->update();
            uint32_t period_ms = 1000 / _refresh_rate;
            usleep(period_ms * 1000);
        }
    #endif

        // 4. destroy
        _destory();
    }

    void _destory()
    {
        destroy();
    #ifdef CONFIG_LIBUV
        uv_close((uv_handle_t*)&async_handle, [](uv_handle_t* handle) {
            appinfo("async handle closed");
        });

        uv_close((uv_handle_t*)&periodic_timer, [](uv_handle_t* handle) {
            appinfo("periodic timer closed");
        });

        uv_stop(loop);
        while (uv_loop_alive(loop)) {
            uv_run(loop, UV_RUN_ONCE);
        }
        uv_loop_close(loop);
        (void)rmdir(CONFIG_DEV_PIPE_VFS_PATH);
    #endif
    }

 private:
    // WiFi AP 配置
    char _ap_ssid[33] = "BOSS1-AP";
    char _ap_password[65] = "12345678";
    in_addr_t _ap_ipaddr;
    in_addr_t _ap_netmask;
    in_addr_t _dhcp_start_ipaddr;
    in_addr_t _dhcp_end_ipaddr;

    // WiFi STA 配置
    char _sta_ssid[33] = "";
    char _sta_password[65] = "";

#ifdef CONFIG_LIBUV
    uv_loop_t* loop = nullptr;
    uint32_t _refresh_rate = 50;  // 默认 50Hz
    uv_timer_t periodic_timer;
    uv_async_t async_handle;

    void periodic_handler() {
        update();
        mc->update();
    }
#endif
    std::unique_ptr<mooncake::Mooncake> mc = nullptr;

public:
    // WiFi AP SSID 和 Password 的 getter/setter
    void set_ap_ssid(const char* ssid) {
        if (ssid) {
            strncpy(_ap_ssid, ssid, sizeof(_ap_ssid) - 1);
            _ap_ssid[sizeof(_ap_ssid) - 1] = '\0';
        }
    }

    const char* get_ap_ssid() const {
        return _ap_ssid;
    }

    void set_ap_password(const char* password) {
        if (password) {
            strncpy(_ap_password, password, sizeof(_ap_password) - 1);
            _ap_password[sizeof(_ap_password) - 1] = '\0';
        }
    }

    const char* get_ap_password() const {
        return _ap_password;
    }

    // WiFi AP IP Address 的 getter/setter
    void set_ap_ipaddr(in_addr_t ipaddr) {
        _ap_ipaddr = ipaddr;
    }

    in_addr_t get_ap_ipaddr() const {
        return _ap_ipaddr;
    }

    // WiFi AP Netmask 的 getter/setter
    void set_ap_netmask(in_addr_t netmask) {
        _ap_netmask = netmask;
    }

    in_addr_t get_ap_netmask() const {
        return _ap_netmask;
    }

    // WiFi AP DHCP 起始 IP 地址的 getter/setter
    void set_dhcp_start_ipaddr(in_addr_t ipaddr) {
        _dhcp_start_ipaddr = ipaddr;
    }

    in_addr_t get_dhcp_start_ipaddr() const {
        return _dhcp_start_ipaddr;
    }

    // WiFi AP DHCP 结束 IP 地址的 getter/setter
    void set_dhcp_end_ipaddr(in_addr_t ipaddr) {
        _dhcp_end_ipaddr = ipaddr;
    }

    in_addr_t get_dhcp_end_ipaddr() const {
        return _dhcp_end_ipaddr;
    }

    // WiFi STA SSID 和 Password 的 getter/setter
    void set_sta_ssid(const char* ssid) {
        if (ssid) {
            strncpy(_sta_ssid, ssid, sizeof(_sta_ssid) - 1);
            _sta_ssid[sizeof(_sta_ssid) - 1] = '\0';
        }
    }

    const char* get_sta_ssid() const {
        return _sta_ssid;
    }

    void set_sta_password(const char* password) {
        if (password) {
            strncpy(_sta_password, password, sizeof(_sta_password) - 1);
            _sta_password[sizeof(_sta_password) - 1] = '\0';
        }
    }

    const char* get_sta_password() const {
        return _sta_password;
    }

#ifdef CONFIG_WIRELESS_WAPI
    void test_wifi_ap_start();
    void test_wifi_sta_connect();
#endif

#ifdef CONFIG_LIBUV
    static void periodic_handler_internal(uv_timer_t* handle) {
        if (!handle || !handle->data) {
            appwarn("Invalid periodic handle or data is null");
            return;
        }

        WatchUI* ui_instance = static_cast<WatchUI*>(handle->data);
        if (!ui_instance) {
            appwarn("Cast to WatchUI failed");
            return;
        }

        ui_instance->periodic_handler();
    }

    static void async_handler_internal(uv_async_t* handle) {
        if (!handle || !handle->data) {
            appwarn("Invalid async handle or data is null");
            return;
        }

        WatchUI* ui_instance = static_cast<WatchUI*>(handle->data);
        if (!ui_instance) {
            appwarn("Cast to WatchUI failed");
            return;
        }

        ui_instance->periodic_handler();
    }

#endif
};

#endif //__WATCHUI_H__
