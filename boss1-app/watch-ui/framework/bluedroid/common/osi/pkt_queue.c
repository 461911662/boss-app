/**
 * @file pkt_queue.c
 * @brief 移植components\bt\common\osi\pkt_queue.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include "osi/pkt_queue.h"
#include "osi/allocator.h"
#include "osi/mutex.h"


/****************************************************************************
 * STRUCTS
 ****************************************************************************/

STAILQ_HEAD(pkt_queue_header, pkt_linked_item);

struct pkt_queue {
    osi_mutex_t lock;
    size_t length;
    struct pkt_queue_header header;
} pkt_queue_t;

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/

/**
 * @details 释放队列中的数据
 * @param queue表示指定的队列
 * @param free_cb表示指定的释放函数
 * @return 无
 */
static void pkt_queue_cleanup(struct pkt_queue *queue, pkt_queue_free_cb free_cb)
{
    if (queue == NULL) {
        return;
    }

    struct pkt_queue_header *header = &queue->header;
    pkt_linked_item_t *item = STAILQ_FIRST(header);
    pkt_linked_item_t *tmp;

    pkt_queue_free_cb free_func = (free_cb != NULL) ? free_cb : (pkt_queue_free_cb)osi_free_func;

    while (item != NULL) {
        tmp = STAILQ_NEXT(item, next);
        free_func(item);
        item = tmp;
        queue->length--;
    }
    STAILQ_INIT(header);
    queue->length = 0;
}

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details 创建一个pkt队列
 * @return 成功时返回pkt_queue，失败时返回NULL
 */
struct pkt_queue *pkt_queue_create(void)
{
    struct pkt_queue *queue = calloc(1, sizeof(struct pkt_queue));
    if (queue == NULL) {
        return NULL;
    }
    if (osi_mutex_new(&queue->lock) != 0) {
        osi_free(queue);
    }
    struct pkt_queue_header *p = &queue->header;
    STAILQ_INIT(p);

    return queue;
}

/**
 * @details 刷新一个pkt队列
 * @param queue表示指定的队列
 * @param free_cb表示指定的释放函数
 * @return 无
 */
void pkt_queue_flush(struct pkt_queue *queue, pkt_queue_free_cb free_cb)
{
    if (queue == NULL) {
        return;
    }
    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    pkt_queue_cleanup(queue, free_cb);
    osi_mutex_unlock(queue->lock);
}

/**
 * @details 销毁一个pkt队列
 * @param queue表示指定的队列
 * @param free_cb表示指定的释放函数
 * @return 无
 */
void pkt_queue_destroy(struct pkt_queue *queue, pkt_queue_free_cb free_cb)
{
    if (queue == NULL) {
        return;
    }
    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    pkt_queue_cleanup(queue, free_cb);
    osi_mutex_unlock(queue->lock);

    osi_mutex_free(&queue->lock);
    osi_free(queue);
}

/**
 * @details 从pkt队列出队
 * @param queue表示指定的队列
 * @return 成功时返回指定的数据，失败时返回NULL
 */
pkt_linked_item_t *pkt_queue_dequeue(struct pkt_queue *queue)
{
    if (queue == NULL || queue->length == 0) {
        return NULL;
    }

    struct pkt_linked_item *item;
    struct pkt_queue_header *header;
    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    header = &queue->header;
    item = STAILQ_FIRST(header);
    if (item != NULL) {
        STAILQ_REMOVE_HEAD(header, next);
        if (queue->length > 0) {
            queue->length--;
        }
    }
    osi_mutex_unlock(queue->lock);

    return item;
}

/**
 * @details 从pkt队列入队
 * @param queue表示指定的队列
 * @param item表示需要入队的数据
 * @return 成功时返回true, 失败时返回false
 */
bool pkt_queue_enqueue(struct pkt_queue *queue, pkt_linked_item_t *item)
{
    if (queue == NULL || item == NULL) {
        return false;
    }

    struct pkt_queue_header *header;
    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    header = &queue->header;
    STAILQ_INSERT_TAIL(header, item, next);
    queue->length++;
    osi_mutex_unlock(queue->lock);

    return true;
}

/**
 * @details 获取pkt队列长度
 * @param queue表示指定的队列
 * @return 成功时返回队列长度, 失败时返回0
 */
size_t pkt_queue_length(const struct pkt_queue *queue)
{
    if (queue == NULL) {
        return 0;
    }
    return queue->length;
}

/**
 * @details 判断pkt队列是否为空
 * @param queue表示指定的队列
 * @return 成功时返回true, 失败时返回false
 */
bool pkt_queue_is_empty(const struct pkt_queue *queue)
{
    return pkt_queue_length(queue) == 0;
}

/**
 * @details 查看pkt队列第一个数据
 * @param queue表示指定的队列
 * @return 成功时返回第一个数据, 失败时返回NULL
 */
pkt_linked_item_t *pkt_queue_try_peek_first(struct pkt_queue *queue)
{
    if (queue == NULL) {
        return NULL;
    }

    struct pkt_queue_header *header = &queue->header;
    pkt_linked_item_t *item;
    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    item = STAILQ_FIRST(header);
    osi_mutex_unlock(queue->lock);

    return item;
}
