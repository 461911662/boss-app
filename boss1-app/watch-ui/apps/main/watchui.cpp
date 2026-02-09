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

static std::unique_ptr<WatchUI> _watchui_instance = nullptr;

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
        }
        DestroyWatchUI();
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
 * @brief 销毁 WatchUI 单例
 */
void DestroyWatchUI() {
    appinfo("destory");
    if (_watchui_instance) {
        _watchui_instance.reset();
    }
}

/**
 * @brief watchui_main 应用入口函数
 */
int watchui_main(int argc, char *argv[]) {
    appdbg("Entry");
    setSignalHandler();
    GetWatchUI().run(); // forever
    DestroyWatchUI();
    return 0;
}

void WatchUI::setup() {
    appdbg("setup");

    get_mooncake().createExtension(std::make_unique<PwmWorker>());
}

void WatchUI::update() {
}

void WatchUI::destroy() {
    appinfo("destroy - cleaning up extensions");
    get_mooncake().resetExtensionManager();
}
