/**
 * @file semaphore.h
 * @brief 移植components\bt\common\osi\include\semaphore.h
 * @attention 可以自由学习
 */

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <sys/types.h>
#include <semaphore.h>

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define OSI_SEM_MAX_TIMEOUT 0xffffffffUL

#define osi_sem_valid( x ) ( ( ( *x ) == NULL) ? FALSE : TRUE )
#define osi_sem_set_invalid( x ) ( ( *x ) = NULL )

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef sem_t* osi_sem_t;


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
/**
 * 创建一个信号量
 * @param sem 表示用于接收创建好的信号量
 * @param pshared 表示信号量的可见范围，0表示线程可见，1表示进程间可见
 * @param init_count 表示提供给信号量的初始值
 * @return 0表示成功，其他表示失败
 */
int osi_sem_new(osi_sem_t *sem, uint32_t pshared, uint32_t init_count);

/**
 * 销毁一个信号量
 * @param sem 表示需要销毁的信号量
 * @return 无
 */
void osi_sem_free(osi_sem_t *sem);

/**
 * 获取一个信号
 * @param sem 表示用于获取信号的信号量
 * @param timeout 表示超时时间
 * @return 0表示成功，其他表示失败
 */
int osi_sem_take(osi_sem_t sem, uint32_t timeout);

/**
 * 释放一个信号
 * @param sem 表示用于释放信号的信号量
 * @return 无
 */
void osi_sem_give(osi_sem_t sem);


#endif /* __SEMAPHORE_H__ */
