/**
 * @file event_bits_group.h
 * @brief 移植components\bt\common\osi\include\osi\event_bits_group.h
 * @attention 可以自由学习
 */


#ifndef _EVENT_BITS_GROUP_H_
#define _EVENT_BITS_GROUP_H_

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <stdio.h>

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/

typedef uint32_t               EventBits_t;
typedef struct event_bits_group EventBitsGroup_S;
typedef EventBitsGroup_S* EventBitsGroupHandle;

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
/**
 * @brief 创建一个EventBitsGroup对象
 * @return 成功时返回EventBitsGroup对象，失败时返回NULL
*/
EventBitsGroupHandle eventBitsGroupCreate(void);

/**
 * @brief 设置eventBitsGroup对象的bit位
 * @param eg 需要设置的eventBitsGroup对象
 * @param bits 需要设置的bits
 * @return 无
*/
void eventBitsGroup_setBits(EventBitsGroupHandle eg, EventBits_t bits);

/**
 * @brief 清除eventBitsGroup对象的bit位
 * @param eg 需要获取的eventBitsGroup对象
 * @param bits 需要清除的bits
 * @return 无
*/
void eventBitsGroup_clearBits(EventBitsGroupHandle eg, EventBits_t bits);

/**
 * @brief 获取eventBitsGroup对象的bit位
 * @param eg 需要获取的eventBitsGroup对象
 * @return 返回获取的EventBits_t
*/
EventBits_t eventBitsGroup_getBits(EventBitsGroupHandle eg);

/**
 * @brief eventBitsGroup阻塞等待bit
 * @param eg 需要阻塞等待的eventBitsGroup对象
 * @param bits_to_wait 需要阻塞获取的bit位
 * @param wait_for_all 是否需要阻塞等待所有要求的bit位
 * @param clear_on_exit 函数返回时，是否清除对应的bit位
 * @param timeout 毫秒级的等待超时时间
 * @return 返回清除前的bit位
*/
EventBits_t eventBitsGroup_waitBits(EventBitsGroupHandle eg, EventBits_t bits_to_wait, int wait_for_all, int clear_on_exit, int timeout);

/**
 * @brief 删除一个EventBitsGroup对象
 * @param eg EventBitsGroup对象的指针
*/
void eventBitsGroup_delete(EventBitsGroupHandle eg);
#endif