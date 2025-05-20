/**
 * @file thread.c
 * @brief 移植components\bt\common\osi\thread.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "osi/allocator.h"
#include "osi/semaphore.h"
#include "osi/thread.h"
#include "osi/mutex.h"

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define BILLION (1000000000L)

/****************************************************************************
 * STRUCTS
 ****************************************************************************/
struct work_item {
    osi_thread_func_t func;
    void *context;
};

struct work_queue {
    mqd_t queue;
    char name[16];
    size_t capacity;
};

struct osi_thread {
  pthread_t thread_handle;           /*!< Store the thread Handle */
  bool stop;
  uint8_t work_queue_num;               /*!< Work queue number */
  struct work_queue **work_queues;      /*!< Point to queue array, and the priority inverse array index */
  osi_sem_t work_sem;
  osi_sem_t stop_sem;
  const char *name;
};

struct osi_thread_start_arg {
  osi_thread_t *thread;
  osi_sem_t start_sem;
  int error;
};

struct osi_event {
    struct work_item item;
    osi_mutex_t lock;
    uint16_t is_queued;
    uint16_t queue_idx;
    osi_thread_t *thread;
};


/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/

static const size_t DEFAULT_WORK_QUEUE_CAPACITY = 100;

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/

/**
 * @details 创建指定大小的队列
 * @param capacity表示队列的大小
 * @return 成功返回队列对象，失败返回NULL
 */
static struct work_queue *osi_work_queue_create(size_t capacity)
{
    if (capacity == 0) {
        return NULL;
    }

    struct work_queue *wq = (struct work_queue *)osi_calloc(sizeof(struct work_queue));
    if (wq != NULL) {
        uintptr_t low32 = (uintptr_t)wq & 0xFFFFFFFF;
        snprintf(wq->name, sizeof(wq->name), "/tmp/%08x", low32);

        struct mq_attr attr;
        attr.mq_maxmsg = capacity;
        attr.mq_msgsize = sizeof(struct work_item);
        attr.mq_curmsgs = 0;
        attr.mq_flags = 0;

        wq->queue = mq_open(wq->name, O_RDWR | O_CREAT, 0644, &attr);
        if (wq->queue < 0)
        {
            osi_free(wq);
            return NULL;
        } else {
            wq->capacity = capacity;
            return wq;
        }
    }

    return NULL;
}

/**
 * @details 删除指定的队列
 * @param wq表示要删除的队列
 * @return 无
 */
static void osi_work_queue_delete(struct work_queue *wq)
{
    if (wq != NULL) {
        mq_close(wq->queue);
        mq_unlink(wq->name);

        wq->queue = 0;
        wq->capacity = 0;
        osi_free(wq);
    }
    return;
}

/**
 * @details 从队列中出队
 * @param wq表示要出队的队列
 * @param item表示接收出队的指针
 * @return 成功返回TRUE，失败返回FALSE
 */
static bool osi_thead_work_queue_get(struct work_queue *wq, struct work_item *item)
{
    assert (wq != NULL);
    assert (wq->queue != 0);
    assert (item != NULL);

    ssize_t ret;
    ret = mq_receive(wq->queue, (char *)item, sizeof(struct work_item), NULL);
    if (ret < 0)
    {
        return FALSE;
    } else {
        return TRUE;
    }
}

/**
 * @details 从队列中入队
 * @param wq表示要入队的队列
 * @param item表示接收出队的指针
 * @param timeout表示队列满时，阻塞的超时时间
 * @return 成功返回TRUE，失败返回FALSE
 */
static bool osi_thead_work_queue_put(struct work_queue *wq, const struct work_item *item, uint32_t timeout)
{
    assert (wq != NULL);
    assert (wq->queue != 0);
    assert (item != NULL);

    bool ret = TRUE;

    if (timeout ==  OSI_SEM_MAX_TIMEOUT) {
        if (mq_send(wq->queue, (const char *)item, sizeof(struct work_item), MQ_PRIO_MAX) < 0) {
            ret = FALSE;
        }
    } else {
        struct timespec now;
        long hz = sysconf(_SC_CLK_TCK);
        if (hz < 0) hz = 1000;
        clock_gettime(CLOCK_REALTIME, &now);

        now.tv_sec += timeout / hz;
        now.tv_nsec += (timeout % hz) * (BILLION / hz);
        if (now.tv_nsec >= BILLION) {
            now.tv_sec += now.tv_nsec / BILLION;
            now.tv_nsec %= BILLION;
        }

        if (mq_timedsend(wq->queue, (const char *)item, sizeof(struct work_item), MQ_PRIO_MAX, &now) < 0) {
            ret = FALSE;
        }
    }

    return ret;
}

/**
 * @details 获取队列等待的长度
 * @param wq表示要入队的队列
 * @return 成功返回队列等待的长度，失败返回小于0的数
 */
static size_t osi_thead_work_queue_len(struct work_queue *wq)
{
    assert (wq != NULL);
    assert (wq->queue != 0);
    assert (wq->capacity != 0);

    struct mq_attr attr;

    if (mq_getattr(wq->queue, &attr) < 0) {
        return -1;
    }

    assert(attr.mq_curmsgs <= wq->capacity);
    return attr.mq_curmsgs;
}

/**
 * @details 线程回调函数
 * @param arg表示线程的回调函数的参数，里面包含线程结构体
 * @return 无
 */
static void *osi_thread_run(void *arg)
{
    struct osi_thread_start_arg *start = (struct osi_thread_start_arg *)arg;
    osi_thread_t *thread = start->thread;

    osi_sem_give(start->start_sem);

    pthread_setname_np(pthread_self(), thread->name);
    while (1) {
        int idx = 0;

        osi_sem_take(thread->work_sem, OSI_SEM_MAX_TIMEOUT);

        if (thread->stop) {
            break;
        }

        struct work_item item;
        while (!thread->stop && idx < thread->work_queue_num) {
            if (osi_thead_work_queue_get(thread->work_queues[idx], &item) == TRUE) {
                item.func(item.context);
                idx = 0;
                continue;
            } else {
                idx++;
            }
        }
    }

    thread->thread_handle = 0;
    osi_sem_give(thread->stop_sem);
    return NULL;
}

/**
 * @details 线程回收函数
 * @param thread表示需要回收的线程
 * @param wait_ms表示需要阻塞等待的超时时间
 * @return 成功返回0，非0表示失败
 */
static int osi_thread_join(osi_thread_t *thread, uint32_t wait_ms)
{
    assert(thread != NULL);
    return osi_sem_take(thread->stop_sem, wait_ms);
}

/**
 * @details 线程停止函数
 * @param thread表示需要停止的线程
 * @return 无
 */
static void osi_thread_stop(osi_thread_t *thread)
{
    int ret;

    assert(thread != NULL);

    //stop the thread
    thread->stop = true;
    osi_sem_give(thread->work_sem);

    //join
    ret = osi_thread_join(thread, 1000); //wait 1000ms

    //if join failed, delete the task here
    if ((ret != 0) && (thread->thread_handle > 0)) {
        pthread_cancel(thread->thread_handle);
    }
}

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details 线程创建函数
 * @param name表示线程的名字
 * @param stack_size表示线程的栈大小
 * @param priority表示线程的优先级
 * @param core表示线程要绑定的核心
 * @param work_queue_num表示线程的队列数量
 * @param work_queue_len表示队列的长度，类型为一个指针数据
 * @return 成功返回线程对象，失败返回NULL
 */
osi_thread_t *osi_thread_create(const char *name, size_t stack_size, int priority, osi_thread_core_t core, uint8_t work_queue_num, const size_t work_queue_len[])
{
    int ret;
    core = core;
    struct osi_thread_start_arg start_arg = {0};

    if (stack_size <= 0 ||
            core < OSI_THREAD_CORE_0 || core > OSI_THREAD_CORE_AFFINITY ||
            work_queue_num <= 0 || work_queue_len == NULL) {
        return NULL;
    }

    osi_thread_t *thread = (osi_thread_t *)osi_calloc(sizeof(osi_thread_t));
    if (thread == NULL) {
        goto _err;
    }

    thread->name = name;
    thread->stop = false;
    thread->work_queues = (struct work_queue **)osi_calloc(sizeof(struct work_queue *) * work_queue_num);
    if (thread->work_queues == NULL) {
        goto _err;
    }
    thread->work_queue_num = work_queue_num;

    for (int i = 0; i < thread->work_queue_num; i++) {
        size_t queue_len = work_queue_len[i] ? work_queue_len[i] : DEFAULT_WORK_QUEUE_CAPACITY;
        thread->work_queues[i] = osi_work_queue_create(queue_len);
        if (thread->work_queues[i] == NULL) {
            goto _err;
        }
    }

    ret = osi_sem_new(&thread->work_sem, 0, 0);
    if (ret != 0) {
        goto _err;
    }

    ret = osi_sem_new(&thread->stop_sem, 0, 0);
    if (ret != 0) {
        goto _err;
    }

    start_arg.thread = thread;
    ret = osi_sem_new(&start_arg.start_sem, 0, 0);
    if (ret != 0) {
        goto _err;
    }

    pthread_attr_t attr;

    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        goto _err;
    }

    ret = pthread_attr_setstacksize(&attr, stack_size);
    if (ret != 0) {
        (void)pthread_attr_destroy(&attr);
        goto _err;
    }

    if (pthread_create(&thread->thread_handle, &attr, osi_thread_run, (pthread_addr_t)(&start_arg)) != 0) {
        (void)pthread_attr_destroy(&attr);
        goto _err;
    }

    ret = pthread_setschedprio(thread->thread_handle, priority);
    if (ret != 0) {
        (void)pthread_attr_destroy(&attr);
        goto _err;
    }

    ret = pthread_attr_destroy(&attr);
    if (ret != 0) {
        goto _err;
    }

    osi_sem_take(start_arg.start_sem, OSI_SEM_MAX_TIMEOUT);
    osi_sem_free(&start_arg.start_sem);

    return thread;

_err:

    if (thread) {
        if (start_arg.start_sem) {
            osi_sem_free(&start_arg.start_sem);
        }

        if (thread->thread_handle > 0) {
            pthread_cancel(thread->thread_handle);
        }

        for (int i = 0; i < thread->work_queue_num; i++) {
            if (thread->work_queues[i]) {
                osi_work_queue_delete(thread->work_queues[i]);
            }
            thread->work_queues[i] = NULL;
        }

        if (thread->work_queues) {
            osi_free(thread->work_queues);
            thread->work_queues = NULL;
        }

        if (thread->work_sem) {
            osi_sem_free(&thread->work_sem);
        }

        if (thread->stop_sem) {
            osi_sem_free(&thread->stop_sem);
        }

        osi_free(thread);
    }

    return NULL;
}

/**
 * @details 线程释放函数
 * @param thread表示要释放的线程
 * @return 无
 */
void osi_thread_free(osi_thread_t *thread)
{
    if (!thread)
        return;

    osi_thread_stop(thread);

    for (int i = 0; i < thread->work_queue_num; i++) {
        if (thread->work_queues[i]) {
            osi_work_queue_delete(thread->work_queues[i]);
            thread->work_queues[i] = NULL;
        }
    }

    if (thread->work_queues) {
        osi_free(thread->work_queues);
        thread->work_queues = NULL;
    }

    if (thread->work_sem) {
        osi_sem_free(&thread->work_sem);
    }

    if (thread->stop_sem) {
        osi_sem_free(&thread->stop_sem);
    }


    osi_free(thread);
}

/**
 * @details 线程提交函数
 * @param thread表示要提交的线程
 * @param func表示线程队列的回调函数
 * @param context表示表示要入队的上下文
 * @param queue_idx表示线程队列的序号
 * @param timeout表示提交阻塞的超时时间
 * @return 无
 */
bool osi_thread_post(osi_thread_t *thread, osi_thread_func_t func, void *context, int queue_idx, uint32_t timeout)
{
    assert(thread != NULL);
    assert(func != NULL);

    if (queue_idx >= thread->work_queue_num) {
        return false;
    }

    struct work_item item;

    item.func = func;
    item.context = context;

    if (osi_thead_work_queue_put(thread->work_queues[queue_idx], &item, timeout) == false) {
        return false;
    }

    osi_sem_give(thread->work_sem);

    return true;
}

/**
 * @details 线程优先级设置函数
 * @param thread表示要设置优先级的线程
 * @param priority表示线程的优先级
 * @return 无
 */
bool osi_thread_set_priority(osi_thread_t *thread, int priority)
{
    assert(thread != NULL);

    pthread_setschedprio(thread->thread_handle, priority);
    return true;
}

/**
 * @details 获取线程名字函数
 * @param thread表示要获取名字的线程
 * @return 成功返回线程名字
 */
const char *osi_thread_name(osi_thread_t *thread)
{
    assert(thread != NULL);

    return thread->name;
}

/**
 * @details 获取指定线程队列等待的长度
 * @param thread表示指定的线程
 * @param wq_idx表示指定的线程队列号
 * @return 成功时返回队列等待的长度，失败时返回小于0的数
 */
int osi_thread_queue_wait_size(osi_thread_t *thread, int wq_idx)
{
    if (wq_idx < 0 || wq_idx >= thread->work_queue_num) {
        return -1;
    }

    return (int)(osi_thead_work_queue_len(thread->work_queues[wq_idx]));
}


/**
 * @details 创建一个事件
 * @param func表示事件的处理函数
 * @param context表示事件的上下文
 * @return 成功返回创建的事件，失败返回NULL
 */
struct osi_event *osi_event_create(osi_thread_func_t func, void *context)
{
    struct osi_event *event = osi_calloc(sizeof(struct osi_event));
    if (event != NULL) {
        if (osi_mutex_new(&event->lock) == 0) {
            event->item.func = func;
            event->item.context = context;
            return event;
        }
        osi_free(event);
    }

    return NULL;
}

/**
 * @details 删除一个事件
 * @param event表示要删除的事件
 * @return 无
 */
void osi_event_delete(struct osi_event* event)
{
    if (event != NULL) {
        osi_mutex_free(&event->lock);
        memset(event, 0, sizeof(struct osi_event));
        osi_free(event);
    }
}

/**
 * @details 将事件绑定到线程中
 * @param event表示要绑定的事件
 * @param thread表示要绑定的线程
 * @param queue_idx表示绑定的线程队列的序号
 * @return 无
 */
bool osi_event_bind(struct osi_event* event, osi_thread_t *thread, int queue_idx)
{
    if (event == NULL || event->thread != NULL) {
        return false;
    }

    if (thread == NULL || queue_idx >= thread->work_queue_num) {
        return false;
    }

    event->thread = thread;
    event->queue_idx = queue_idx;

    return true;
}

/**
 * 线程事件回调函数
 * @param context表示事件对象
 * @return 无
 */
static void osi_thread_generic_event_handler(void *context)
{
    struct osi_event *event = (struct osi_event *)context;
    if (event != NULL && event->item.func != NULL) {
        osi_mutex_lock(event->lock, OSI_MUTEX_MAX_TIMEOUT);
        event->is_queued = 0;
        osi_mutex_unlock(event->lock);
        event->item.func(event->item.context);
    }
}

/**
 * 给绑定的线程提交事件
 * @param event表示需要提交的事件
 * @param timeout表示提交时由于线程队列满而阻塞的超时时间
 * @return 成功时返回TRUE，失败时返回FALSE
 */
bool osi_thread_post_event(struct osi_event *event, uint32_t timeout)
{
    assert(event != NULL && event->thread != NULL);
    assert(event->queue_idx >= 0 && event->queue_idx < event->thread->work_queue_num);
    bool ret = false;
    if (event->is_queued == 0) {
        uint16_t acquire_cnt = 0;
        osi_mutex_lock(event->lock, OSI_MUTEX_MAX_TIMEOUT);
        event->is_queued += 1;
        acquire_cnt = event->is_queued;
        osi_mutex_unlock(event->lock);

        if (acquire_cnt == 1) {
            ret = osi_thread_post(event->thread, osi_thread_generic_event_handler, event, event->queue_idx, timeout);
            if (!ret) {
                // clear "is_queued" when post failure, to allow for following event posts
                osi_mutex_lock(event->lock, OSI_MUTEX_MAX_TIMEOUT);
                event->is_queued = 0;
                osi_mutex_unlock(event->lock);
            }
        }
    }

    return ret;
}
