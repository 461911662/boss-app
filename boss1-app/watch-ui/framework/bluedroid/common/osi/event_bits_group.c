/**
 * @file event_bit_group.c
 * @brief 移植components\bt\common\osi\event_bit_group.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include "bt_osi_mem.h"
#include "osi/event_bits_group.h"

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/


/****************************************************************************
 * STRUCTS
 ****************************************************************************/
struct event_bits_group {
    EventBits_t event_bits;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
/*
 * @details 创建一个EventBitsGroup对象
 * @return 成功时返回创建的对象指针，失败时返回NULL
 */
EventBitsGroupHandle eventBitsGroupCreate(void)
{
    EventBitsGroupHandle eg = (EventBitsGroupHandle)osi_calloc(1, sizeof(EventBitsGroup_S));
    if(eg != NULL) {
       eg->event_bits = 0;
       pthread_mutex_init(&eg->lock, NULL);
       pthread_cond_init(&eg->cond, NULL);
    }

    return eg;
}

/*
 * @details eventBitsGroup设置bit
 */
void eventBitsGroup_setBits(EventBitsGroupHandle eg, EventBits_t bits) {
    assert(eg != NULL);

    pthread_mutex_lock(&eg->lock);
    eg->event_bits |= bits;
    pthread_cond_broadcast(&eg->cond);
    pthread_mutex_unlock(&eg->lock);
}

/*
 * @details  eventBitsGroup清除bit
 */
void eventBitsGroup_clearBits(EventBitsGroupHandle eg, EventBits_t bits) {
    assert(eg != NULL);

    pthread_mutex_lock(&eg->lock);
    eg->event_bits &= ~bits;
    pthread_cond_broadcast(&eg->cond);
    pthread_mutex_unlock(&eg->lock);
}

/*
 * @details  eventBitsGroup获取bit
 */
EventBits_t eventBitsGroup_getBits(EventBitsGroupHandle eg) {
    assert(eg != NULL);

    EventBits_t bits;
    pthread_mutex_lock(&eg->lock);
    bits = eg->event_bits;
    pthread_mutex_unlock(&eg->lock);
    return bits;
}

/*
 * @details  eventBitsGroup阻塞等待bit
 */
EventBits_t eventBitsGroup_waitBits(EventBitsGroupHandle eg, EventBits_t bits_to_wait, int wait_for_all, int clear_on_exit, int timeout) {
    assert(eg != NULL);

    pthread_mutex_lock(&eg->lock);
    EventBits_t prev_bits = 0;
    while (1) {
        if ((wait_for_all && ((eg->event_bits&bits_to_wait) == bits_to_wait)) ||
            (!wait_for_all && (eg->event_bits&bits_to_wait))) {
            prev_bits = eg->event_bits;
            if (clear_on_exit) {
                eg->event_bits &= ~bits_to_wait;
            }
            break;
        }

        if (timeout > 0) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000;
            }

            int ret = pthread_cond_timedwait(&eg->cond, &eg->lock, &ts);
            if (ret == ETIMEDOUT) {
                prev_bits = eg->event_bits;
                break;
            }
        } else {
            pthread_cond_wait(&eg->cond, &eg->lock);
        }
    }

    pthread_mutex_unlock(&eg->lock);
    return prev_bits;
}

/*
 * @details  销毁eventBitsGroup对象
 */
void eventBitsGroup_delete(EventBitsGroupHandle eg) {
    assert(eg != NULL);

    pthread_mutex_destroy(&eg->lock);
    pthread_cond_destroy(&eg->cond);
    osi_free(eg);
}