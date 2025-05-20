/**
 * @file mutex.h
 * @brief 移植components\bt\common\osi\include\osi\mutex.h
 * @attention 可以自由学习
 */


#ifndef __MUTEX_H__
#define __MUTEX_H__

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <pthread.h>
#include <sys/types.h>
#include <nuttx/clock.h>

#include "osi/semaphore.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define OSI_MUTEX_MAX_TIMEOUT OSI_SEM_MAX_TIMEOUT

#define osi_mutex_valid( x ) ( ( ( *x ) == NULL) ? FALSE : TRUE )
#define osi_mutex_set_invalid( x ) ( ( *x ) = NULL )


/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef pthread_mutex_t* osi_mutex_t;


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
/** 创建mutex锁
 * @param mutex 传入一个mutex指针，用于接收创建好的mutex对象
 * @return 0表示成功，其他表示失败 */
int osi_mutex_new(osi_mutex_t *mutex);

/** mutex上锁
 * @param mutex 传入一个mutex指针，进行上锁
 * @param timeout 表示上锁的超时时间，如果timeout为OSI_MUTEX_MAX_TIMEOUT时，表示阻塞上锁
 * @return 0表示成功，其他表示失败 */
int osi_mutex_lock(osi_mutex_t mutex, uint32_t timeout);

/** mutex解锁
 * @param mutex 传入一个mutex指针，进行解锁
 * @return 无 */
void osi_mutex_unlock(osi_mutex_t mutex);

/** 释放mutex锁
 * @param mutex 传入一个mutex指针，用于释放mutex对象
 * @return 无 */
void osi_mutex_free(osi_mutex_t *mutex);

/** 初始化全局mutex锁
 * @return 0表示成功，其他表示失败 */
int osi_mutex_global_init(void);

/** 解初始化全局mutex锁
 * @return 无 */
void osi_mutex_global_deinit(void);

/** 全局mutex上锁
 * @return 无 */
void osi_mutex_global_lock(void);

/** 全局mutex解锁
 * @return 无 */
void osi_mutex_global_unlock(void);

#endif /* __MUTEX_H__ */
