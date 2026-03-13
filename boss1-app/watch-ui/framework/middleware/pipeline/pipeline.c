/*
 * watch-ui/framework/middleware/pipeline/pipeline.c
 *
 * Pipeline implementation with libuv integration
 */

#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <uv.h>
#include <nuttx/queue.h>
#include "pipeline.h"
#include <watchui/log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_PIPELINE_DEBUG
#  define PIPELINE_LOG(fmt, ...)    appinfo("[Pipeline] " fmt, ##__VA_ARGS__)
#  define PIPELINE_DBG(fmt, ...)    appdbg("[Pipeline] " fmt, ##__VA_ARGS__)
#  define PIPELINE_WARN(fmt, ...)   appwarn("[Pipeline] " fmt, ##__VA_ARGS__)
#  define PIPELINE_ERR(fmt, ...)    apperr("[Pipeline] " fmt, ##__VA_ARGS__)
#else
#  define PIPELINE_LOG(fmt, ...)
#  define PIPELINE_DBG(fmt, ...)
#  define PIPELINE_WARN(fmt, ...)   appwarn("[Pipeline] " fmt, ##__VA_ARGS__)
#  define PIPELINE_ERR(fmt, ...)    apperr("[Pipeline] " fmt, ##__VA_ARGS__)
#endif

static pipeline_manager_t g_pipeline_manager = {0};

/* Forward declarations */
static void pipeline_timeout_cb(uv_timer_t *handle);
static void pipeline_process_current(pipeline_t *p, int event_id, void *event_data);

int pipeline_manager_init(uv_loop_t *loop)
{
    g_pipeline_manager.loop = loop;
    uv_mutex_init(&g_pipeline_manager.lock);
    g_pipeline_manager.pipelines = NULL;
    return 0;
}

pipeline_t *pipeline_create(const char *name, uint32_t timeout_ms,
                             void (*on_complete)(pipeline_t *, bool, void *),
                             void *user_data)
{
    pipeline_t *p = (pipeline_t *)calloc(1, sizeof(pipeline_t));
    if (!p) {
        return NULL;
    }

    p->name = name;
    p->timeout_ms = timeout_ms;
    p->on_complete = on_complete;
    p->user_data = user_data;
    p->loop = g_pipeline_manager.loop;
    p->active = false;
    p->head = NULL;
    p->current = NULL;
    p->next = NULL;

    if (!p->loop) {
        free(p);
        return NULL;
    }

    /* Initialize timeout timer */
    uv_timer_init(p->loop, &p->timeout_timer);
    p->timeout_timer.data = p;

    return p;
}

/* Check if node's wait event list is empty */
bool node_wait_event_is_empty(pipeline_node_t *node)
{
    return sq_empty(&node->wait_events);
}

/* Helper function to add a wait event to node */
static int node_add_wait_event(pipeline_node_t *node, int event_id)
{
    struct node_wait_event *evt = (struct node_wait_event *)malloc(sizeof(struct node_wait_event));
    if (!evt) {
        return -ENOMEM;
    }
    
    evt->event_id = event_id;
    sq_addlast(&evt->entry, &node->wait_events);
    return 0;
}

/* Check if event is in node's wait list */
static bool node_is_waiting_for_event(pipeline_node_t *node, int event_id)
{
    if (sq_empty(&node->wait_events)) {
        return (event_id == NO_WAIT_EVENT);
    }
    
    sq_entry_t *entry;
    sq_for_every(&node->wait_events, entry) {
        if (((struct node_wait_event *)entry)->event_id == event_id) {
            return true;
        }
    }
    return false;
}

/* Free node's wait events */
static void node_free_wait_events(pipeline_node_t *node)
{
    struct node_wait_event *evt;
    while ((evt = (struct node_wait_event *)sq_remfirst(&node->wait_events)) != NULL) {
        free(evt);
    }
}

/* Add node with multiple wait events (variable arguments) 
 * Usage: pipeline_add_node_v(p, "wait_start", func, ctx, 2, 4, 5); 
 *        waits for event 4 or 5
 *        pipeline_add_node_v(p, "validate", func, ctx, 0); 
 *        no wait, execute immediately
 */
int pipeline_add_node_v(pipeline_t *p, const char *name,
                         pipeline_node_func_t func, void *ctx, 
                         int wait_event_count, ...)
{
    if (!p || !func) {
        return -EINVAL;
    }

    pipeline_node_t *node = (pipeline_node_t *)calloc(1, sizeof(pipeline_node_t));
    if (!node) {
        return -ENOMEM;
    }

    node->name = name;
    node->func = func;
    node->ctx = ctx;
    node->next = NULL;
    node->pipeline = p;  /* Back link to parent pipeline */
    node->triggered_event_id = -1;
    sq_init(&node->wait_events);

    /* Add wait events from variadic arguments */
    va_list args;
    va_start(args, wait_event_count);
    
    for (int i = 0; i < wait_event_count; i++) {
        int event_id = va_arg(args, int);
        if (node_add_wait_event(node, event_id) < 0) {
            node_free_wait_events(node);
            free(node);
            va_end(args);
            return -ENOMEM;
        }
    }
    va_end(args);

    /* Add to end of list */
    if (!p->head) {
        p->head = node;
    } else {
        pipeline_node_t *n = p->head;
        while (n->next) {
            n = n->next;
        }
        n->next = node;
    }

    return 0;
}

/* Backward compatible wrapper - single event */
int pipeline_add_node(pipeline_t *p, const char *name,
                       pipeline_node_func_t func, void *ctx, int wait_event_id)
{
    if (wait_event_id == NO_WAIT_EVENT) {
        return pipeline_add_node_v(p, name, func, ctx, 0);
    } else {
        return pipeline_add_node_v(p, name, func, ctx, 1, wait_event_id);
    }
}

int pipeline_start(pipeline_t *p)
{
    if (!p || !p->head) {
        return -EINVAL;
    }

    p->active = true;
    p->current = p->head;

    /* Start timeout timer */
    uv_timer_start(&p->timeout_timer, pipeline_timeout_cb, p->timeout_ms, 0);

    /* Add to manager list */
    uv_mutex_lock(&g_pipeline_manager.lock);
    p->next = g_pipeline_manager.pipelines;
    g_pipeline_manager.pipelines = p;
    uv_mutex_unlock(&g_pipeline_manager.lock);

    /* Execute first node immediately */
    pipeline_process_current(p, NO_WAIT_EVENT, NULL);

    return 0;
}

static void pipeline_process_current(pipeline_t *p, int event_id, void *event_data)
{
    if (!p || !p->current || !p->active) {
        return;
    }

    /* Pass node self as parameter, so node function can access node->pipeline, node->name, etc. */
    node_result_t result = p->current->func(p->current, event_id, event_data);

    switch (result) {
    case NODE_SUCCESS:
        /* Move to next node */
        p->current = p->current->next;
        if (p->current) {
            /* Execute next node immediately */
            pipeline_process_current(p, NO_WAIT_EVENT, NULL);
        } else {
            /* Pipeline completed successfully */
            p->active = false;
            uv_timer_stop(&p->timeout_timer);
            if (p->on_complete) {
                p->on_complete(p, true, p->user_data);
            }
        }
        break;

    case NODE_FAILED:
        /* Pipeline failed */
        p->active = false;
        uv_timer_stop(&p->timeout_timer);
        if (p->on_complete) {
            p->on_complete(p, false, p->user_data);
        }
        break;

    case NODE_WAITING:
        /* Waiting for event, do nothing */
        break;
    }
}

static void pipeline_timeout_cb(uv_timer_t *handle)
{
    pipeline_t *p = (pipeline_t *)handle->data;

    if (p && p->active) {
        p->active = false;
        if (p->on_complete) {
            p->on_complete(p, false, p->user_data);
        }
    }
}

static void pipeline_destroy(pipeline_t *p)
{
    if (!p) {
        return;
    }

    /* Remove from manager list */
    uv_mutex_lock(&g_pipeline_manager.lock);
    pipeline_t **pp = &g_pipeline_manager.pipelines;
    while (*pp) {
        if (*pp == p) {
            *pp = p->next;
            break;
        }
        pp = &(*pp)->next;
    }
    uv_mutex_unlock(&g_pipeline_manager.lock);

    /* Mark as inactive first to prevent timer callback from executing */
    p->active = false;

    /* Stop and close timer */
    uv_timer_stop(&p->timeout_timer);
    uv_close((uv_handle_t *)&p->timeout_timer, NULL);

    /* Free nodes */
    pipeline_node_t *node = p->head;
    while (node) {
        pipeline_node_t *next = node->next;
        node_free_wait_events(node);
        free(node);
        node = next;
    }
    
    /* Free pipeline */
    free(p);
}

void pipeline_handle_event(int event_id, void *event_data)
{
    uv_mutex_lock(&g_pipeline_manager.lock);

    pipeline_t *p = g_pipeline_manager.pipelines;
    while (p) {
        if (p->active && p->current) {
            /* Check if current node is waiting for this event */
            if (node_is_waiting_for_event(p->current, event_id)) {
                /* Mark triggered event ID and execute node */
                p->current->triggered_event_id = event_id;
                pipeline_process_current(p, event_id, event_data);
            }
        }
        p = p->next;
    }

    uv_mutex_unlock(&g_pipeline_manager.lock);
}

/* Async destroy callback */
static void pipeline_async_destroy_cb(uv_async_t *handle)
{
    pipeline_t *p = (pipeline_t *)handle->data;
    if (p) {
        pipeline_destroy(p);
    }
    uv_close((uv_handle_t *)handle, NULL);
    handle->data = NULL;
    free(handle);
}

/* Async destroy - safe to call from libuv callbacks
 * Uses uv_async to delay destruction until next loop tick
 */
void pipeline_destroy_async(pipeline_t *p)
{
    if (!p || !p->loop) {
        return;
    }
    
    /* Allocate async handle on heap */
    uv_async_t *async_handle = (uv_async_t *)malloc(sizeof(uv_async_t));
    if (!async_handle) {
        return;
    }
    
    int ret = uv_async_init(p->loop, async_handle, pipeline_async_destroy_cb);
    if (ret < 0) {
        free(async_handle);
        return;
    }
    
    async_handle->data = p;
    uv_async_send(async_handle);
}
