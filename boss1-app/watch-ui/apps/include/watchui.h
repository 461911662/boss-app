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
    }
    ~WatchUI() {
        if (mc) {
            mc.reset();
        }
    }

    Mooncake& get_mooncake() { return *mc; };

#ifdef CONFIG_LIBUV
    void set_loop_cnt(int32_t cnt) {
        uv_timer_stop(&heartbeat_req);

        heartbeat_cnt = cnt;
        if (cnt != 0) {
            uv_timer_start(&heartbeat_req, heartbeat_handler_internal, 0, 1000);
        }
    }
    uv_loop_t* get_loop() { return loop; }
    uv_async_t* get_async_handle() { return &async_update_handle; }
#endif

    void setup();
    void update();
    void destroy();

    void run() {
        // 1. pre setup
    #ifdef CONFIG_LIBUV
        loop = uv_default_loop();
        async_update_handle.data = this;
        int ret = uv_async_init(loop, &async_update_handle, async_handler_internal);
        if (ret != 0) {
            apperr("uv_async_init failed, ret: %d", ret);
            return;
        }
        heartbeat_req.data = this;
        ret = uv_timer_init(loop, &heartbeat_req);
        if (ret != 0) {
            apperr("uv_timer_init failed, ret: %d", ret);
            return;
        }
    #endif

        // 2. setup
        setup();

        // 3. loop
    #ifdef CONFIG_LIBUV
        if (heartbeat_cnt != 0) {
            uv_timer_start(&heartbeat_req, heartbeat_handler_internal, 0, 1000);
        }
        uv_run(loop, UV_RUN_DEFAULT);
    #else
        while(1) {
            update();
            mc->update();
            sleep(1);
        }
    #endif

        // 4. destroy
        _destory();
    }

    void _destory()
    {
        destroy();
    #ifdef CONFIG_LIBUV
        uv_close((uv_handle_t*)&async_update_handle, [](uv_handle_t* handle) {
            appinfo("Async handle closed");
        });

        uv_close((uv_handle_t*)&heartbeat_req, [](uv_handle_t* handle) {
            appinfo("Heartbeat handle closed");
        });

        uv_stop(loop);
        while (uv_loop_alive(loop)) {
            uv_run(loop, UV_RUN_ONCE);
        }
        uv_loop_close(loop);
    #endif
    }

private:
#ifdef CONFIG_LIBUV
    uv_loop_t* loop = nullptr;
    int32_t heartbeat_cnt = -1; // forever loop
    uv_timer_t heartbeat_req;
    uv_async_t async_update_handle;

    void heartbeat() {
        if (heartbeat_cnt == -1 || heartbeat_cnt > 0)
        {
            (void)uv_async_send(&async_update_handle);

            if (heartbeat_cnt > 0) {
                heartbeat_cnt--;
            }
            if (heartbeat_cnt == 0) {
                uv_timer_stop(&heartbeat_req);
            }
        }
    }
#endif
    std::unique_ptr<mooncake::Mooncake> mc = nullptr;

#ifdef CONFIG_LIBUV
    static void heartbeat_handler_internal(uv_timer_t* handle) {
        if (!handle || !handle->data) {
            appwarn("Invalid heartbeat handle or data is null");
            return;
        }

        WatchUI* ui_instance = static_cast<WatchUI*>(handle->data);
        if (!ui_instance) {
            appwarn("Cast to WatchUI failed");
            return;
        }

        ui_instance->heartbeat();
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

        ui_instance->update();
        ui_instance->get_mooncake().update();
    }
#endif
};

#endif //__WATCHUI_H__
