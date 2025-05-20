/**
 * @file future.c
 * @brief 移植谷歌开源软件
 * @attention 请遵循开源软件协议
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include "bt_common.h"
#include "osi/allocator.h"
#include "osi/future.h"
#include "osi/osi.h"

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/

void future_free(future_t *future);

future_t *future_new(void)
{
    future_t *ret = osi_calloc(sizeof(future_t));
    if (!ret) {
        OSI_TRACE_ERROR("%s unable to allocate memory for return value.", __func__);
        goto error;
    }

    if (osi_sem_new(&ret->semaphore, 0, 0) != 0) {
        OSI_TRACE_ERROR("%s unable to allocate memory for the semaphore.", __func__);
        goto error;
    }

    ret->ready_can_be_called = true;
    return ret;
error:;
    future_free(ret);
    return NULL;
}

future_t *future_new_immediate(void *value)
{
    future_t *ret = osi_calloc(sizeof(future_t));
    if (!ret) {
        OSI_TRACE_ERROR("%s unable to allocate memory for return value.", __func__);
        goto error;
    }

    ret->result = value;
    ret->ready_can_be_called = false;
    return ret;
error:;
    future_free(ret);
    return NULL;
}

void future_ready(future_t *future, void *value)
{
    assert(future != NULL);
    assert(future->ready_can_be_called);

    future->ready_can_be_called = false;
    future->result = value;
    osi_sem_give(future->semaphore);
}

void *future_await(future_t *future)
{
    assert(future != NULL);

    // If the future is immediate, it will not have a semaphore
    if (future->semaphore) {
        osi_sem_take(future->semaphore, OSI_SEM_MAX_TIMEOUT);
    }

    void *result = future->result;
    future_free(future);
    return result;
}

void future_free(future_t *future)
{
    if (!future) {
        return;
    }

    if (future->semaphore) {
        osi_sem_free(&future->semaphore);
    }

    osi_free(future);
}
