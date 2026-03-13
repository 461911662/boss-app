/****************************************************************************
 * watch-ui/framework/middleware/pipeline/pipeline.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __WATCH_UI_FRAMEWORK_MIDDLEWARE_PIPELINE_H
#define __WATCH_UI_FRAMEWORK_MIDDLEWARE_PIPELINE_H

#include <nuttx/config.h>
#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Node execution result */
typedef enum {
    NODE_SUCCESS = 0,   /* Node completed successfully, proceed to next */
    NODE_FAILED,        /* Node failed, terminate pipeline */
    NODE_WAITING,       /* Node waiting for async event, suspend */
} node_result_t;

/* Forward declarations */
struct pipeline_node;
struct pipeline;

#define NO_WAIT_EVENT (-1) /* pipeline node don't need to wait event or is first call. */

/* Node function type - node can access self and pipeline context */
typedef node_result_t (*pipeline_node_func_t)(struct pipeline_node *node,
                                               int event_id, 
                                               void *event_data);

/* Node wait event entry - for multiple event waiting */
struct node_wait_event {
    sq_entry_t entry;
    int event_id;               /* Event ID to wait for */
};

/* Pipeline node structure */
typedef struct pipeline_node {
    const char *name;
    pipeline_node_func_t func;
    void *ctx;                  /* Node's user context */
    struct pipeline_node *next;
    struct pipeline *pipeline;  /* Back link to parent pipeline */
    
    /* Event waiting support - can wait for multiple events */
    sq_queue_t wait_events;     /* List of events this node is waiting for */
    int triggered_event_id;     /* The event ID that triggered this node (-1 if not triggered) */
} pipeline_node_t;

/* Pipeline structure */
typedef struct pipeline {
    const char *name;
    pipeline_node_t *head;
    pipeline_node_t *current;
    
    /* libuv handles */
    uv_timer_t timeout_timer;
    uv_loop_t *loop;
    
    /* State */
    bool active;
    uint32_t timeout_ms;
    
    /* Callbacks */
    void (*on_complete)(struct pipeline *p, bool success, void *user_data);
    void *user_data;
    
    /* Linked list */
    struct pipeline *next;
} pipeline_t;

/* Pipeline Manager */
typedef struct {
    uv_loop_t *loop;
    uv_mutex_t lock;              /* For libuv thread synchronization */
    pipeline_t *pipelines;
} pipeline_manager_t;

/* API Functions */
int pipeline_manager_init(uv_loop_t *loop);
pipeline_t *pipeline_create(const char *name, uint32_t timeout_ms,
                             void (*on_complete)(pipeline_t *, bool, void *),
                             void *user_data);

/* Add node with multiple wait events (variable arguments)
 * Usage: pipeline_add_node_v(p, "wait_start", func, ctx, 2, 4, 5);  // waits for event 4 or 5
 *        pipeline_add_node_v(p, "validate", func, ctx, 0);  // no wait, execute immediately
 */
int pipeline_add_node_v(pipeline_t *p, const char *name,
                         pipeline_node_func_t func, void *ctx,
                         int wait_event_count, ...);

/* Backward compatible wrapper - single event */
int pipeline_add_node(pipeline_t *p, const char *name,
                       pipeline_node_func_t func, void *ctx, int wait_event_id);

int pipeline_start(pipeline_t *p);

/* Async destroy - safe to call from libuv callbacks
 * Uses uv_async to delay destruction until next loop tick
 */
void pipeline_destroy_async(pipeline_t *p);

/* Check if node's wait event list is empty */
bool node_wait_event_is_empty(pipeline_node_t *node);

/* Event handling */
void pipeline_handle_event(int event_id, void *event_data);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_UI_FRAMEWORK_MIDDLEWARE_PIPELINE_H */
