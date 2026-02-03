/**
 * @file watchui/apps/include/watchui.h
 * @brief watchui 类头文件
*/

#ifndef __WATCHUI_H__
#define __WATCHUI_H__

/* Global enums -------------------------------------------------------------- */
/* Global structs ------------------------------------------------------------ */
/* Global typedefs ----------------------------------------------------------- */
/* Global variables ---------------------------------------------------------- */
/* Global variable prototypes ------------------------------------------------ */
/* Global function prototypes ------------------------------------------------ */

/* Includes ------------------------------------------------------------------ */

#include "ability/ability.h"

/* Global defines ------------------------------------------------------------ */
namespace mooncake {

class WatchUI {

public:

    WatchUI() {
        mc = std::make_unique<Mooncake>();
        setup();
    };

    ~WatchUI() {
        destroy();
    };

    // 1. 初始化流程的函数 setup
    void setup();

    // 2. 更新流程的函数 update
    void update();

    // 3. 销毁流程的函数 destroy
    void destroy();

private:

    std::unique_ptr<Mooncake> mc = nullptr; //Mooncake m_mooncake;
};

}

#endif //__WATCHUI_H__
