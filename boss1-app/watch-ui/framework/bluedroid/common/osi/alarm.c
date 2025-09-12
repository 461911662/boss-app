/**
 * @file alarm.c
 * @brief 移植components\bt\common\osi\alarm.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

#include "osi/alarm.h"
#include "osi/allocator.h"
#include "osi/list.h"
#include "btc/btc_task.h"
#include "btc/btc_alarm.h"
#include "osi/mutex.h"
#include "bt_common.h"


/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef struct alarm_t {
    const char* alarm_name;
    osi_alarm_callback_t alarm_cb;
    osi_alarm_callback_t cb;
    void *cb_data;
    int64_t deadline;
    bool is_periodic;
    period_ms_t timeout;
} osi_alarm_t;


/****************************************************************************
 * ENUMS
 ****************************************************************************/
enum {
    ALARM_STATE_IDLE,
    ALARM_STATE_OPEN,
};


/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/
static osi_mutex_t alarm_mutex;
static int alarm_state;

#if (BT_BLE_DYNAMIC_ENV_MEMORY == FALSE)
static struct alarm_t alarm_cbs[ALARM_CBS_NUM];
#else
static struct alarm_t *alarm_cbs;
#endif
static list_t *alarms;
static timer_t timer;
static bool timer_set;

int64_t TIMER_INTERVAL_FOR_WAKELOCK_IN_MS = 3000;
static const clockid_t CLOCK_ID = CLOCK_BOOTTIME;

static void reschedule(void);
static period_ms_t now(void);
static void alarm_cb_handler(void *data);
static struct alarm_t *alarm_cbs_lookfor_available(void);
static osi_alarm_err_t alarm_free(osi_alarm_t *alarm);
static osi_alarm_err_t alarm_set(osi_alarm_t *alarm, period_ms_t timeout, bool is_periodic);

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

/**
 * @details 创建闹钟互斥锁
 * @return 成功返回0，失败返回1 */
int osi_alarm_create_mux(void)
{
    if (alarm_state != ALARM_STATE_IDLE) {
        OSI_TRACE_WARNING("%s, invalid state %d\n", __func__, alarm_state);
        return -1;
    }
    osi_mutex_new(&alarm_mutex);
    return 0;
}

/**
 * @details 删除闹钟互斥锁
 * @return 成功返回0, 失败返回1 */
int osi_alarm_delete_mux(void)
{
    if (alarm_state != ALARM_STATE_IDLE) {
        OSI_TRACE_WARNING("%s, invalid state %d\n", __func__, alarm_state);
        return -1;
    }
    osi_mutex_free(&alarm_mutex);
    return 0;
}

/**
 * @details 初始化osi的闹钟设备
 * 需要注意如下条件：
 *  1. 在调用osi_alarm_init之前，必须先创建alarm_mutex
 *  2. alarm_state这个状态必须是ALARM_STATE_IDLE
 */
void osi_alarm_init(void)
{
    assert(alarm_mutex != NULL);

    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm_state != ALARM_STATE_IDLE) {
        OSI_TRACE_WARNING("%s, invalid state %d\n", __func__, alarm_state);
        goto end;
    }
#if (BT_BLE_DYNAMIC_ENV_MEMORY == TRUE)
    if ((alarm_cbs = (osi_alarm_t *)osi_malloc(sizeof(osi_alarm_t) * ALARM_CBS_NUM)) == NULL) {
        OSI_TRACE_ERROR("%s, malloc failed\n", __func__);
        goto end;
    }
#endif

    memset(alarm_cbs, 0x00, sizeof(osi_alarm_t) * ALARM_CBS_NUM);
    alarms = osi_list_new(NULL);
    alarm_state = ALARM_STATE_OPEN;

end:
    osi_mutex_unlock(alarm_mutex);
}

/**
 * @details 解初始化osi的闹钟设备
 * 需要注意如下条件：
 *  1. 在调用osi_alarm_deinit之前，必须先创建alarm_mutex
 *  2. alarm_state这个状态必须是ALARM_STATE_OPEN
 */
void osi_alarm_deinit(void)
{
    assert(alarm_mutex != NULL);

    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm_state != ALARM_STATE_OPEN) {
        OSI_TRACE_WARNING("%s, invalid state %d\n", __func__, alarm_state);
        goto end;
    }

    for (int i = 0; i < ALARM_CBS_NUM; i++) {
        if (alarm_cbs[i].alarm_cb != NULL) {
            alarm_free(&alarm_cbs[i]);
        }
    }

#if (BT_BLE_DYNAMIC_ENV_MEMORY == TRUE)
    osi_free(alarm_cbs);
    alarm_cbs = NULL;
#endif

    osi_list_free(alarms);
    alarm_state = ALARM_STATE_IDLE;

end:
    osi_mutex_unlock(alarm_mutex);
}

/**
 * @details 创建一个osi闹钟定时器
 * 需要注意如下条件：
 *  1. 在调用osi_alarm_deinit之前，必须先创建alarm_mutex
 *  2. alarm_state这个状态必须是ALARM_STATE_OPEN
 *  3. 需要注意当前闹钟创建的定时器有没有达到上限
 */
osi_alarm_t *osi_alarm_new(const char *alarm_name, osi_alarm_callback_t callback, void *data, period_ms_t timer_expire)
{
    assert(alarm_mutex != NULL);

    struct alarm_t *timer_id = NULL;

    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm_state != ALARM_STATE_OPEN) {
        OSI_TRACE_ERROR("%s, invalid state %d\n", __func__, alarm_state);
        timer_id = NULL;
        goto end;
    }

    timer_id = alarm_cbs_lookfor_available();
    if (!timer_id) {
        OSI_TRACE_ERROR("%s alarm_cbs exhausted\n", __func__);
        timer_id = NULL;
        goto end;
    }

    memset(timer_id, 0, sizeof(osi_alarm_t));
    timer_id->alarm_name = alarm_name;
    timer_id->alarm_cb = (osi_alarm_callback_t)alarm_cb_handler;
    timer_id->cb = callback;
    timer_id->cb_data = data;
    timer_id->deadline = 0;

end:
    osi_mutex_unlock(alarm_mutex);
    return timer_id;
}

/**
 * @details 释放一个osi闹钟定时器
 * 需要注意如下条件：
 *  1. 在调用osi_alarm_deinit之前，必须先创建alarm_mutex
 *  2. alarm_state这个状态必须是ALARM_STATE_OPEN
 */
void osi_alarm_free(osi_alarm_t *alarm)
{
    assert(alarm_mutex != NULL);

    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm_state != ALARM_STATE_OPEN) {
        OSI_TRACE_ERROR("%s, invalid state %d\n", __func__, alarm_state);
        goto end;
    }
    alarm_free(alarm);

end:
    osi_mutex_unlock(alarm_mutex);
    return;
}

/**
 * @details 设置osi闹钟定时器的超时时间
 */
osi_alarm_err_t osi_alarm_set(osi_alarm_t *alarm, period_ms_t timeout)
{
    return alarm_set(alarm, timeout, FALSE);
}

/**
 * @details 设置osi闹钟定时器为周期定时器
 */
osi_alarm_err_t osi_alarm_set_periodic(osi_alarm_t *alarm, period_ms_t period)
{
    return alarm_set(alarm, period, TRUE);
}

/**
 * @details 取消osi闹钟定时器
 */
osi_alarm_err_t osi_alarm_cancel(osi_alarm_t *alarm)
{
    int ret = OSI_ALARM_ERR_PASS;
    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm_state != ALARM_STATE_OPEN) {
        OSI_TRACE_ERROR("%s, invalid state %d\n", __func__, alarm_state);
        ret = OSI_ALARM_ERR_INVALID_STATE;
        goto end;
    }

    if (!alarm || alarm->alarm_cb == NULL) {
        OSI_TRACE_ERROR("%s null\n", __func__);
        ret = OSI_ALARM_ERR_INVALID_ARG;
        goto end;
    }

    bool is_schedule = (!osi_list_is_empty(alarms) && osi_list_front(alarms) == alarm);

    osi_list_remove(alarms, alarm);

    if (is_schedule) {
        reschedule();
    }

end:
    osi_mutex_unlock(alarm_mutex);
    return ret;
}

/**
 * @details 获取osi闹钟定时器的剩余时间
 */
period_ms_t osi_alarm_get_remaining_ms(const osi_alarm_t *alarm)
{
    assert(alarm_mutex != NULL);
    int64_t dt_us = 0;

    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    dt_us = alarm->deadline - now();
    osi_mutex_unlock(alarm_mutex);

    return (dt_us > 0) ? (period_ms_t)(dt_us / 1000) : 0;
}

/**
 * @details 获取当前的启动时间，单位为毫秒
 */
uint32_t osi_time_get_os_boottime_ms(void)
{
    return (uint32_t)(now() / 1000);
}

/**
 * @details 获取osi闹钟定时器是否在运行
 */
bool osi_alarm_is_active(osi_alarm_t *alarm)
{
    assert(alarm != NULL);

    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm->alarm_cb != NULL) {
        osi_mutex_unlock(alarm_mutex);
        return TRUE;
    }

    osi_mutex_unlock(alarm_mutex);
    return FALSE;
}

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/
/**
 * @details 获取osi闹钟有效的定时器
 */
static struct alarm_t *alarm_cbs_lookfor_available(void)
{
    int i;

    for (i = 0; i < ALARM_CBS_NUM; i++) {
        if (alarm_cbs[i].alarm_cb == NULL) { //available
            OSI_TRACE_DEBUG("%s %d %p\n", __func__, i, &alarm_cbs[i]);
            return &alarm_cbs[i];
        }
    }

    return NULL;
}

/**
 * @details 获取osi闹钟定时器的回调函数
 */
static void alarm_cb_handler(void *data)
{
    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    struct alarm_t *alarm = (struct alarm_t *)data;
    OSI_TRACE_DEBUG("TimerName: %s\n", alarm->alarm_name);
    if (alarm_state != ALARM_STATE_OPEN) {
        OSI_TRACE_WARNING("%s, invalid state %d\n", __func__, alarm_state);
        return;
    }

    bool alarm_valid = osi_list_remove(alarms, alarm);

    if (alarm_valid && alarm->is_periodic) {
        alarm->deadline = now() + alarm->timeout;
        if (osi_list_is_empty(alarms)) {
            osi_list_prepend(alarms, alarm);
        } else {
            for (list_node_t *node = osi_list_begin(alarms); node != osi_list_end(alarms); node = osi_list_next(node)) {
              list_node_t *next = osi_list_next(node);
              if (next == osi_list_end(alarms) || ((osi_alarm_t *)osi_list_node(next))->deadline >= alarm->deadline) {
                osi_list_insert_after(alarms, node, alarm);
                break;
              }
            }
        }
    }

    reschedule();

    // 定时器已经取消了
    if (!alarm_valid) {
      osi_mutex_unlock(alarm_mutex);
      return;
    }

    btc_msg_t msg = {0};
    btc_alarm_args_t arg;
    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_ALARM;
    arg.cb = alarm->cb;
    arg.cb_data = alarm->cb_data;

    if (!alarm->is_periodic) {
        alarm->alarm_cb = NULL;
    }
    osi_mutex_unlock(alarm_mutex);

    btc_transfer_context_from_alarm(&msg, &arg, sizeof(btc_alarm_args_t), NULL, NULL);
}

/**
 * @details 释放一个内部的osi闹钟定时器
 * @note 这个函数操作全局变量，必须在上层加锁
 */
static osi_alarm_err_t alarm_free(osi_alarm_t *alarm)
{
    if (!alarm) {
        OSI_TRACE_ERROR("%s null\n", __func__);
        return OSI_ALARM_ERR_INVALID_ARG;
    }

    // 停止该闹钟定时器
    bool is_schedule = (!osi_list_is_empty(alarms) && osi_list_front(alarms) == alarm);

    osi_list_remove(alarms, alarm);

    if (is_schedule) {
        reschedule();
    }

    memset(alarm, 0, sizeof(osi_alarm_t));
    return OSI_ALARM_ERR_PASS;
}

/**
 * @details 获取当前系统的时间
 */
static period_ms_t now(void) {
    assert(alarms != NULL);

    struct timespec ts;
    if (clock_gettime(CLOCK_ID, &ts) == -1) {
        OSI_TRACE_ERROR("%s unable to get current time: %s", __func__, strerror(errno));
        return 0;
    }

    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
}

/**
 * @details 定时器调度函数
 */
#ifdef CONFIG_SIG_EVTHREAD
static void reschedule(void) {
    assert(alarms != NULL);

    // 判断定时器在使用，先删除之前的定时器，然后再重新启动定时器
    if (timer_set) {
        timer_delete(timer);
        timer_set = false;
    }

    if (osi_list_is_empty(alarms)) {
        // TODO: 添加WAKE LOCK方案来允许系统休眠
        return;
    }

    osi_alarm_t *next = (osi_alarm_t *)osi_list_front(alarms);
    int64_t next_exp = next->deadline - now();

    if (next_exp < TIMER_INTERVAL_FOR_WAKELOCK_IN_MS) {
        // TODO: 使用WAKE LOCK方案来禁止系统休眠
        struct sigevent sigevent;
        memset(&sigevent, 0, sizeof(sigevent));
        sigevent.sigev_notify = SIGEV_THREAD;
        sigevent.sigev_notify_function = (void (*)(union sigval))next->alarm_cb;
        sigevent.sigev_value.sival_ptr = next;
        if (timer_create(CLOCK_ID, &sigevent, &timer) == -1) {
            OSI_TRACE_ERROR("%s unable to create timer: %s", __func__, strerror(errno));
            return;
        }

        struct itimerspec wakeup_time;
        memset(&wakeup_time, 0, sizeof(wakeup_time));
        wakeup_time.it_value.tv_sec = (next->deadline / 1000);
        wakeup_time.it_value.tv_nsec = (next->deadline % 1000) * 1000000LL;
        if (timer_settime(timer, TIMER_ABSTIME, &wakeup_time, NULL) == -1) {
            OSI_TRACE_ERROR("%s unable to set timer: %s", __func__, strerror(errno));
            timer_delete(timer);
            return;
        }
    } else {
        // TODO: 使用WAKE LOCK方案来允许系统休眠
        struct sigevent sigevent;
        memset(&sigevent, 0, sizeof(sigevent));
        sigevent.sigev_notify = SIGEV_THREAD;
        sigevent.sigev_notify_function = (void (*)(union sigval))next->alarm_cb;
        sigevent.sigev_value.sival_ptr = next;
        if (timer_create(CLOCK_MONOTONIC, &sigevent, &timer) == -1) {
            OSI_TRACE_ERROR("%s unable to create timer: %s", __func__, strerror(errno));
            return;
        }

        struct itimerspec wakeup_time;
        memset(&wakeup_time, TIMER_ABSTIME, sizeof(wakeup_time));
        wakeup_time.it_value.tv_sec = (next->deadline / 1000);
        wakeup_time.it_value.tv_nsec = (next->deadline % 1000) * 1000000LL;
        if (timer_settime(timer, TIMER_ABSTIME, &wakeup_time, NULL) == -1) {
            OSI_TRACE_ERROR("%s unable to set timer: %s", __func__, strerror(errno));
            timer_delete(timer);
            return;
        }
    }

    timer_set = true;
}
#else
static void reschedule(void) {
}
#endif

/**
 * @details 闹钟定时器设置函数
 */
static osi_alarm_err_t alarm_set(osi_alarm_t *alarm, period_ms_t timeout, bool is_periodic)
{
    assert(alarm_mutex != NULL);
    assert(alarms != NULL);
    assert(alarm != NULL);

    osi_alarm_err_t ret = OSI_ALARM_ERR_PASS;
    osi_mutex_lock(alarm_mutex, OSI_MUTEX_MAX_TIMEOUT);
    if (alarm_state != ALARM_STATE_OPEN) {
        OSI_TRACE_ERROR("%s, invalid state %d\n", __func__, alarm_state);
        ret = OSI_ALARM_ERR_INVALID_STATE;
        goto end;
    }

    if (!alarm || alarm->alarm_cb == NULL) {
        OSI_TRACE_ERROR("%s null\n", __func__);
        ret = OSI_ALARM_ERR_INVALID_ARG;
        goto end;
    }

    // 处理设置多次alarm的情况, 正好在计时
    bool needs_reschedule = (!osi_list_is_empty(alarms) && osi_list_front(alarms) == alarm);

    // 处理设置多次alarm的情况, 还未计时
    bool is_contains = osi_list_contains(alarms, alarm);
    if (needs_reschedule || is_contains) {
        osi_list_remove(alarms, alarm);
    }

    alarm->timeout = timeout;
    alarm->is_periodic = is_periodic;
    alarm->deadline = now() + timeout;
    if (osi_list_is_empty(alarms)) {
        osi_list_prepend(alarms, alarm);
    } else {
        for (list_node_t *node = osi_list_begin(alarms); node != osi_list_end(alarms); node = osi_list_next(node)) {
          list_node_t *next = osi_list_next(node);
          if (next == osi_list_end(alarms) || ((osi_alarm_t *)osi_list_node(next))->deadline >= alarm->deadline) {
            osi_list_insert_after(alarms, node, alarm);
            break;
          }
        }
    }

    if (needs_reschedule || (!osi_list_is_empty(alarms) && osi_list_front(alarms) == alarm)) {
        reschedule();
    }

end:
    osi_mutex_unlock(alarm_mutex);
    return ret;
}
