/**
 * @file alarm.h
 * @brief 移植components\bt\common\osi\include\osi\alarm.h
 * @attention 可以自由学习
 */


#ifndef _ALARM_H_
#define _ALARM_H_


/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "bt_user_config.h"


/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef struct alarm_t osi_alarm_t;
typedef uint64_t period_ms_t;
typedef void (*osi_alarm_callback_t)(void *data);

typedef enum {
    OSI_ALARM_ERR_PASS = 0,
    OSI_ALARM_ERR_FAIL = -1,
    OSI_ALARM_ERR_INVALID_ARG = -2,
    OSI_ALARM_ERR_INVALID_STATE = -3,
} osi_alarm_err_t;


/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define ALARM_CBS_NUM   UC_ALARM_MAX_NUM


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
int osi_alarm_create_mux(void);
int osi_alarm_delete_mux(void);
void osi_alarm_init(void);
void osi_alarm_deinit(void);

// Creates a new alarm object. The returned object must be freed by calling
// |alarm_free|. Returns NULL on failure.
osi_alarm_t *osi_alarm_new(const char *alarm_name, osi_alarm_callback_t callback, void *data, period_ms_t timer_expire);

// Frees an alarm object created by |alarm_new|. |alarm| may be NULL. If the
// alarm is pending, it will be cancelled. It is not safe to call |alarm_free|
// from inside the callback of |alarm|.
void osi_alarm_free(osi_alarm_t *alarm);

// Sets an alarm to fire |cb| after the given |deadline|. Note that |deadline| is the
// number of milliseconds relative to the current time. |data| is a context variable
// for the callback and may be NULL. |cb| will be called back in the context of an
// unspecified thread (i.e. it will not be called back in the same thread as the caller).
// |alarm| and |cb| may not be NULL.
osi_alarm_err_t osi_alarm_set(osi_alarm_t *alarm, period_ms_t timeout);

// Sets an periodic alarm to fire |cb| each given |period|.
osi_alarm_err_t osi_alarm_set_periodic(osi_alarm_t *alarm, period_ms_t period);

// This function cancels the |alarm| if it was previously set. When this call
// returns, the caller has a guarantee that the callback is not in progress and
// will not be called if it hasn't already been called. This function is idempotent.
// |alarm| may not be NULL.
osi_alarm_err_t osi_alarm_cancel(osi_alarm_t *alarm);

// Figure out how much time until next expiration.
// Returns 0 if not armed. |alarm| may not be NULL.
// only for oneshot alarm, not for periodic alarm
// TODO: Remove this function once PM timers can be re-factored
period_ms_t osi_alarm_get_remaining_ms(const osi_alarm_t *alarm);

// Alarm-related state cleanup
//void alarm_cleanup(void);

uint32_t osi_time_get_os_boottime_ms(void);

// This function returns whether the given |alarm| is active or not.
// Return true if active, false otherwise.
bool osi_alarm_is_active(osi_alarm_t *alarm);

#endif /*_ALARM_H_*/
