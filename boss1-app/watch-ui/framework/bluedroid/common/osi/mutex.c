/**
 * @file mutex.c
 * @brief 移植components\bt\common\osi\mutex.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include "osi/mutex.h"
#include "bt_osi_mem.h"

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static pthread_mutex_t *gl_mutex; /* Recursive Type */


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/

/**
 * @details Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex */
int osi_mutex_new(osi_mutex_t *mutex)
{
    int xReturn = -1;

    *mutex = (osi_mutex_t)osi_malloc(sizeof(pthread_mutex_t));
    if (*mutex == NULL) {
        return 0;
    }

    memset(*mutex, 0, sizeof(pthread_mutex_t));
    int ret = pthread_mutex_init(*mutex, NULL);
    if (ret) {
        osi_free(*mutex);
        *mutex = NULL;
    }

    if (*mutex != NULL) {
        xReturn = 0;
    }

    return xReturn;
}

/**
 * @details Lock a mutex
 * @param mutex the mutex to lock 
 * @param timeout 上锁超时时间，单位为系统滴答次数
 */
int osi_mutex_lock(osi_mutex_t mutex, uint32_t timeout)
{
    int ret = 0;

    if (timeout == OSI_MUTEX_MAX_TIMEOUT) {
        if (pthread_mutex_lock(mutex)) {
            ret = -1;
        }
    } else {
        struct timespec time;
        clock_ticks2time(&time, timeout);
        if (pthread_mutex_timedlock(mutex, &time))  {
            ret = -2;
        }
    }

    return ret;
}

/**
 * @details Unlock a mutex
 * @param mutex the mutex to unlock */
void osi_mutex_unlock(osi_mutex_t mutex)
{
    pthread_mutex_unlock(mutex);
}

/**
 * @details Delete a semaphore
 * @param mutex the mutex to delete */
void osi_mutex_free(osi_mutex_t *mutex)
{
    pthread_mutex_destroy(*mutex);
    osi_free(*mutex);
    *mutex = NULL;
}

/**
 * @details 初始化全局互斥锁
 * @return 成功返回0，失败返回非零 */
int osi_mutex_global_init(void)
{
    int ret;

    if (gl_mutex != NULL) {
        return -1;
    }

    pthread_mutexattr_t attr;
#ifdef CONFIG_PTHREAD_MUTEX_TYPES
    ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (ret) {
        return -2;
    }
#endif

    ret = pthread_mutexattr_init(&attr);
    if (ret) {
        pthread_mutexattr_destroy(&attr);
        return -3;
    }

    gl_mutex = osi_malloc(sizeof(pthread_mutex_t));
    if (!gl_mutex) {
        pthread_mutexattr_destroy(&attr);
        return -4;
    }

    memset(gl_mutex, 0, sizeof(pthread_mutex_t));
    ret = pthread_mutex_init(gl_mutex, &attr);
    if (ret) {
        pthread_mutexattr_destroy(&attr);
        return -5;
    }

    return 0;
}

/**
 * @details 注销全局互斥锁
 */
void osi_mutex_global_deinit(void)
{
    pthread_mutex_destroy(gl_mutex);
    osi_free(gl_mutex);
    gl_mutex = NULL;
}

/**
 * @details 全局互斥锁上锁
 */
void osi_mutex_global_lock(void)
{
    pthread_mutex_lock(gl_mutex);
}

/**
 * @details 全局互斥锁解锁
 */
void osi_mutex_global_unlock(void)
{
    pthread_mutex_unlock(gl_mutex);
}
