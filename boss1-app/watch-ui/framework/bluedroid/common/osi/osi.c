/**
 * @file osi.c
 * @brief 移植components\bt\common\osi\osi.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include "osi/osi.h"
#include "osi/mutex.h"


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/

/**
 * @details osi接口初始化函数
 */
int osi_init(void)
{
    int ret = 0;

    if (osi_mutex_global_init() != 0) {
        ret = -1;
    }

    return ret;
}

/**
 * @details osi接口解初始化函数
 */
void osi_deinit(void)
{
    osi_mutex_global_deinit();
}
