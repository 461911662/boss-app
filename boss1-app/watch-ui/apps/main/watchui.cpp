/**
 * @file watchui/apps/main/watchui.cpp
 * @brief watchui 类实现文件
*/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <cstdio>
#include <iostream>

#ifdef CONFIG_LIBUV
#include <uv.h>
#else
#include <unistd.h>
#endif

#include <mooncake.h>

#include "watchui.h"
#include "watchui/log.h"
#include "watchui_c2cxx.h"

#ifdef CONFIG_LIBUV
static uv_loop_t *loop;
static uv_async_t async;
#endif

void mooncake::WatchUI::setup()
{
}

void mooncake::WatchUI::update()
{
}

void mooncake::WatchUI::destroy()
{
}

/**
 * @brief: watchui_main 应用入口函数
 * @param: argc 参数数量
 * @param: argv 参数数组
 * @return: 0
*/
int watchui_main(int argc, char *argv[]) {
    appinfo("Entry:\n");

    mooncake::WatchUI watchui = mooncake::WatchUI();
#ifdef CONFIG_LIBUV
    loop = uv_default_loop();

    uv_async_init(loop, &async, watchui.update);

    return uv_run(loop, UV_RUN_DEFAULT);
#else
    while(1) {
        watchui.update();
        sleep(1);
    }
#endif
}
