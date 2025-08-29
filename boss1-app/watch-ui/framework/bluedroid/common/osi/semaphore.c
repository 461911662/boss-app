/**
 * @file semaphore.c
 * @brief 移植components\bt\common\osi\semaphore.c
 * @attention 可以自由学习
 */


/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <errno.h>
#include "osi/semaphore.h"
#include "osi/allocator.h"

#define MS_PER_SECOND  (1000)
#define NS_PER_MSECOND (1000000)
#define NS_PER_SECOND  (1000000000)


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/

/**
 * @details osi_sem_new需要提供内容:
 * 1、存放申请的信号量的指针
 * 2、信号量的可见度
 * 3、信号量的初始可用信号数
 */
int osi_sem_new(osi_sem_t *sem, uint32_t pshared, uint32_t init_count)
{
    if (sem == NULL) {
        return -EINVAL;
    }

    *sem = osi_malloc(sizeof(sem_t));
    if (*sem == NULL) {
        return -ENOMEM;
    }

    int ret = sem_init(*sem, pshared, init_count);
    if (ret < 0) {
        return -ret;
    }

    return 0;
}

/**
 * @details osi_sem_give需要提供内容:
 * 1、释放的信号的信号量指针
 */
void osi_sem_give(osi_sem_t sem)
{
    if (sem == NULL) {
        return;
    }

    sem_post(sem);
}

/**
 * @details osi_sem_take需要提供内容:
 * 1、获取的信号的信号量指针
 * 2、指定超时时间，如果超时将直接返回，否则会一直阻塞，单位毫秒
 */
int osi_sem_take(osi_sem_t sem, uint32_t timeout)
{
    if (sem == NULL) {
        return -EINVAL;
    }

    int status = 0;
    struct timespec ts;
    int wait_time = (timeout == OSI_SEM_MAX_TIMEOUT) ? -1 : timeout;

    if (wait_time != -1) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += (time_t)(wait_time / MS_PER_SECOND);
        ts.tv_nsec += (long)((wait_time % MS_PER_SECOND) * NS_PER_MSECOND);
        if (ts.tv_nsec >= NS_PER_SECOND) {
            ts.tv_sec += ts.tv_nsec / NS_PER_SECOND;
            ts.tv_nsec = ts.tv_nsec % NS_PER_SECOND;
        }
        status = sem_timedwait(sem, &ts);
    } else {
        status = sem_wait(sem);
    }

    return (status == OK) ? 0 : -EAGAIN; // Return 0 on success, non-zero on failure
}

/**
 * @details osi_sem_free需要提供内容:
 * 1、释放信号量的指针
 */
void osi_sem_free(osi_sem_t *sem)
{
    if (sem == NULL) {
        return;
    }
    
    sem_destroy(*sem);
    osi_free(*sem);
    *sem = NULL;    
}
