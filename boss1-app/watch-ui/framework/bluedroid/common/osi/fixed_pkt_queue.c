/**
 * @file fixed_pkt_queue.c
 * @brief 移植components\bt\common\osi\fixed_pkt_queue.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <assert.h>

#include "osi/allocator.h"
#include "osi/pkt_queue.h"
#include "osi/fixed_pkt_queue.h"
#include "osi/osi.h"
#include "osi/semaphore.h"

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/

typedef struct fixed_pkt_queue_t {
    struct pkt_queue *pkt_list;
    osi_sem_t enqueue_sem;
    osi_sem_t dequeue_sem;
    size_t capacity;
    fixed_pkt_queue_cb dequeue_ready;
} fixed_pkt_queue_t;

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details 创建固定大小的fixed_pkt_queue
 * @param capacity表示队列大小
 * @return 成功时返回创建的fixed_pkt_queue对象，失败时返回NULL
 */
fixed_pkt_queue_t *fixed_pkt_queue_new(size_t capacity)
{
    fixed_pkt_queue_t *ret = osi_calloc(sizeof(fixed_pkt_queue_t));
    if (!ret) {
        goto error;
    }

    ret->capacity = capacity;
    ret->pkt_list = pkt_queue_create();
    if (!ret->pkt_list) {
        goto error;
    }

    osi_sem_new(&ret->enqueue_sem, 0, capacity);
    if (!ret->enqueue_sem) {
        goto error;
    }

    osi_sem_new(&ret->dequeue_sem, 0, 0);
    if (!ret->dequeue_sem) {
        goto error;
    }

    return ret;

error:
    fixed_pkt_queue_free(ret, NULL);
    return NULL;
}

/**
 * @details 释放指定的fixed_pkt_queue
 * @param queue指定的队列对象
 * @param free_cb表示释放队列数据的回调
 * @return 无
 */
void fixed_pkt_queue_free(fixed_pkt_queue_t *queue, fixed_pkt_queue_free_cb free_cb)
{
    if (queue == NULL) {
        return;
    }

    fixed_pkt_queue_unregister_dequeue(queue);

    pkt_queue_destroy(queue->pkt_list, (pkt_queue_free_cb)free_cb);
    queue->pkt_list = NULL;

    if (queue->enqueue_sem) {
        osi_sem_free(&queue->enqueue_sem);
    }
    if (queue->dequeue_sem) {
        osi_sem_free(&queue->dequeue_sem);
    }
    osi_free(queue);
}

/**
 * @details 判断fixed_pkt_queue是否为空
 * @param queue指定的队列对象
 * @return 成功返回true,失败返回false
 */
bool fixed_pkt_queue_is_empty(fixed_pkt_queue_t *queue)
{
    if (queue == NULL) {
        return true;
    }

    return pkt_queue_is_empty(queue->pkt_list);
}

/**
 * @details 获取fixed_pkt_queue的长度
 * @param queue指定的队列对象
 * @return 成功返回fixed_pkt_queue的长度,失败返回0
 */
size_t fixed_pkt_queue_length(fixed_pkt_queue_t *queue)
{
    if (queue == NULL) {
        return 0;
    }
    return pkt_queue_length(queue->pkt_list);
}

/**
 * @details 获取fixed_pkt_queue的大小
 * @param queue指定的队列对象
 * @return 成功返回fixed_pkt_queue的大小,失败返回0
 */
size_t fixed_pkt_queue_capacity(fixed_pkt_queue_t *queue)
{
    assert(queue != NULL);

    return queue->capacity;
}

/**
 * @details fixed_pkt_queue的入队
 * @param queue指定的队列对象
 * @return 成功返回true,失败返回false
 */
bool fixed_pkt_queue_enqueue(fixed_pkt_queue_t *queue, pkt_linked_item_t *linked_pkt, uint32_t timeout)
{
    bool ret = false;

    assert(queue != NULL);
    assert(linked_pkt != NULL);

    if (osi_sem_take(queue->enqueue_sem, timeout) != 0) {
        return false;
    }

    ret = pkt_queue_enqueue(queue->pkt_list, linked_pkt);

    assert(ret == true);
    osi_sem_give(queue->dequeue_sem);

    return ret;
}

/**
 * @details fixed_pkt_queue的出队
 * @param queue指定的队列对象
 * @param timeout表示出队的超时时间
 * @return 成功返回pkt_linked_item_t对象,失败返回NULL
 */
pkt_linked_item_t *fixed_pkt_queue_dequeue(fixed_pkt_queue_t *queue, uint32_t timeout)
{
    pkt_linked_item_t *ret = NULL;

    assert(queue != NULL);

    if (osi_sem_take(queue->dequeue_sem, timeout) != 0) {
        return NULL;
    }
    ret = pkt_queue_dequeue(queue->pkt_list);

    osi_sem_give(queue->enqueue_sem);

    return ret;
}

/**
 * @details 查看fixed_pkt_queue的第一个数据
 * @param queue指定的队列对象
 * @return 成功返回pkt_linked_item_t对象,失败返回NULL
 */
pkt_linked_item_t *fixed_pkt_queue_try_peek_first(fixed_pkt_queue_t *queue)
{
    if (queue == NULL) {
        return NULL;
    }

    return pkt_queue_try_peek_first(queue->pkt_list);
}

/**
 * @details 注册fixed_pkt_queue的回调函数
 * @param queue指定的队列对象
 * @param ready_cb表示fixed_pkt_queue的回调函数
 * @return 无
 */
void fixed_pkt_queue_register_dequeue(fixed_pkt_queue_t *queue, fixed_pkt_queue_cb ready_cb)
{
    assert(queue != NULL);
    assert(ready_cb != NULL);

    queue->dequeue_ready = ready_cb;
}

/**
 * @details 注销fixed_pkt_queue的回调函数
 * @param queue指定的队列对象
 * @return 无
 */
void fixed_pkt_queue_unregister_dequeue(fixed_pkt_queue_t *queue)
{
    assert(queue != NULL);

    queue->dequeue_ready = NULL;
}

/**
 * @details fixed_pkt_queue的回调函数处理函数
 * @param queue指定的队列对象
 * @return 无
 */
void fixed_pkt_queue_process(fixed_pkt_queue_t *queue)
{
    assert(queue != NULL);

    if (queue->dequeue_ready) {
        queue->dequeue_ready(queue);
    }
}
