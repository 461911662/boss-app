/**
 * @file fixed_queue.c
 * @brief 移植谷歌开源软件
 * @attention 请遵循开源软件协议
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include "osi/allocator.h"
#include "osi/fixed_queue.h"
#include "osi/list.h"
#include "osi/osi.h"
#include "osi/mutex.h"
#include "osi/semaphore.h"

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/

typedef struct fixed_queue_t {

    list_t *list;
    osi_sem_t enqueue_sem;
    osi_sem_t dequeue_sem;
    osi_mutex_t lock;
    size_t capacity;

    fixed_queue_cb dequeue_ready;
} fixed_queue_t;

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details 创建一个固定队列
 * @param capacity表示队列的大小
 * @return 成功时返回队列对象，失败时返回NULL
 */
fixed_queue_t *fixed_queue_new(size_t capacity)
{
    fixed_queue_t *ret = osi_calloc(sizeof(fixed_queue_t));
    if (!ret) {
        goto error;
    }

    osi_mutex_new(&ret->lock);
    ret->capacity = capacity;

    ret->list = osi_list_new(NULL);
    if (!ret->list) {
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

error:;
    fixed_queue_free(ret, NULL);
    return NULL;
}

/**
 * @details 释放一个固定队列
 * @param queue表示需要释放的队列对象
 * @param free_cb表示释放队列数据的回调函数
 * @return 无
 */
void fixed_queue_free(fixed_queue_t *queue, fixed_queue_free_cb free_cb)
{
    const list_node_t *node;

    if (queue == NULL) {
	    return;
	}

    fixed_queue_unregister_dequeue(queue);

    if (free_cb) {
        for (node = osi_list_begin(queue->list); node != osi_list_end(queue->list); node = osi_list_next(node)) {
            free_cb(osi_list_node(node));
        }
    }

    osi_list_free(queue->list);
    osi_sem_free(&queue->enqueue_sem);
    osi_sem_free(&queue->dequeue_sem);
    osi_mutex_free(&queue->lock);
    osi_free(queue);
}

/**
 * @details 判断固定队列是否为空
 * @param queue表示需要指定的队列对象
 * @return 成功时返回true，失败时返回false
 */
bool fixed_queue_is_empty(fixed_queue_t *queue)
{
    bool is_empty = false;

    if (queue == NULL) {
        return true;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    is_empty = osi_list_is_empty(queue->list);
    osi_mutex_unlock(queue->lock);

    return is_empty;
}

/**
 * @details 获取固定队列当前的长度
 * @param queue表示需要指定的队列对象
 * @return 成功时返回固定队列当前长度，失败时返回0
 */
size_t fixed_queue_length(fixed_queue_t *queue)
{
    size_t length;

    if (queue == NULL) {
        return 0;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    length = osi_list_length(queue->list);
    osi_mutex_unlock(queue->lock);

    return length;
}

/**
 * @details 获取固定队列的长度
 * @param queue表示需要指定的队列对象
 * @return 成功时返回固定队列长度，失败时返回小于0
 */
size_t fixed_queue_capacity(fixed_queue_t *queue)
{
    assert(queue != NULL);

    return queue->capacity;
}

/**
 * @details 固定队列入队
 * @param queue表示需要指定的队列对象
 * @param data表示入队的数据
 * @param timeout表示入队时的超时时间，如果超时则返回
 * @return 成功时返回true，失败时返回false
 */
bool fixed_queue_enqueue(fixed_queue_t *queue, void *data, uint32_t timeout)
{
    bool status=false; //Flag whether enqueued success

    assert(queue != NULL);
    assert(data != NULL);

    if (osi_sem_take(queue->enqueue_sem, timeout) != 0) {
        return false;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    status = osi_list_append(queue->list, data); //Check whether enqueued success
    osi_mutex_unlock(queue->lock);

    if(status == true )
        osi_sem_give(queue->dequeue_sem);

    return status;
}

/**
 * @details 固定队列出队
 * @param queue表示需要指定的队列对象
 * @param data表示入队的数据
 * @param timeout表示出队时的超时时间，如果超时则返回
 * @return 成功时返回出队的数据，失败时返回NULL
 */
void *fixed_queue_dequeue(fixed_queue_t *queue, uint32_t timeout)
{
    void *ret = NULL;

    assert(queue != NULL);

    if (osi_sem_take(queue->dequeue_sem, timeout) != 0) {
        return NULL;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    ret = osi_list_front(queue->list);
    osi_list_remove(queue->list, ret);
    osi_mutex_unlock(queue->lock);

    osi_sem_give(queue->enqueue_sem);

    return ret;
}

/**
 * @details 查看队列中第一个数据
 * @param queue表示需要指定的队列对象
 * @return 成功时返回第一个数据，失败时返回NULL
 */
void *fixed_queue_try_peek_first(fixed_queue_t *queue)
{
    void *ret = NULL;

    if (queue == NULL) {
        return NULL;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    ret = osi_list_is_empty(queue->list) ? NULL : osi_list_front(queue->list);
    osi_mutex_unlock(queue->lock);

    return ret;
}

/**
 * @details 查看队列中最后一个数据
 * @param queue表示需要指定的队列对象
 * @return 成功时返回最后一个数据，失败时返回NULL
 */
void *fixed_queue_try_peek_last(fixed_queue_t *queue)
{
    void *ret = NULL;

    if (queue == NULL) {
        return NULL;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    ret = osi_list_is_empty(queue->list) ? NULL : osi_list_back(queue->list);
    osi_mutex_unlock(queue->lock);

    return ret;
}

/**
 * @details 从固定队列中移除指定的数据
 * @param queue表示需要指定的队列对象
 * @param data表示需要从队列中移除的数据
 * @return 成功时返回移除的数据，失败时返回NULL
 */
void *fixed_queue_try_remove_from_queue(fixed_queue_t *queue, void *data)
{
    bool removed = false;

    if (queue == NULL) {
        return NULL;
    }

    osi_mutex_lock(queue->lock, OSI_MUTEX_MAX_TIMEOUT);
    if (osi_list_contains(queue->list, data) &&
            osi_sem_take(queue->dequeue_sem, 0) == 0) {
        removed = osi_list_remove(queue->list, data);
        assert(removed);
    }
    osi_mutex_unlock(queue->lock);

    if (removed) {
        osi_sem_give(queue->enqueue_sem);
        return data;
    }

    return NULL;
}

/**
 * @details 从固定队列中获取队列列表
 * @param queue表示需要指定的队列对象
 * @return 成功时返回队列列表，失败时返回NULL
 * @note 这个函数这个函数线程不安全
 */
list_t *fixed_queue_get_list(fixed_queue_t *queue)
{
    assert(queue != NULL);

    // NOTE: This function is not thread safe, and there is no point for
    // calling osi_mutex_lock() / osi_mutex_unlock()
    return queue->list;
}

/**
 * @details 注册队列处理函数
 * @param queue表示需要指定的队列对象
 * @param ready_cb表示传递的回调函数
 * @return 无
 */
void fixed_queue_register_dequeue(fixed_queue_t *queue, fixed_queue_cb ready_cb)
{
    assert(queue != NULL);
    assert(ready_cb != NULL);

    queue->dequeue_ready = ready_cb;
}

/**
 * @details 注销队列处理函数
 * @param queue表示需要指定的队列对象
 * @return 无
 */
void fixed_queue_unregister_dequeue(fixed_queue_t *queue)
{
    assert(queue != NULL);

    queue->dequeue_ready = NULL;
}

/**
 * @details 调用队列处理函数
 * @param queue表示需要指定的队列对象
 * @return 无
 */
void fixed_queue_process(fixed_queue_t *queue)
{
    assert(queue != NULL);

    if (queue->dequeue_ready) {
        queue->dequeue_ready(queue);
    }
}
