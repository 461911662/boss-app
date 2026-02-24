/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi_manager/pipeline_sta_connect.c
 *
 * STA Connect Pipeline Implementation
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#include "wifi_manager.h"
#include "wifi.h"
#include "pipeline.h"
#include "watchui/log.h"

/****************************************************************************
 * Logging Macros
 ****************************************************************************/
#define painfo(fmt, ...)    appinfo("[PipelineStaStart] " fmt, ##__VA_ARGS__)
#define padbg(fmt, ...)     appdbg("[PipelineStaStart] " fmt, ##__VA_ARGS__)
#define pawarn(fmt, ...)    appwarn("[PipelineStaStart] " fmt, ##__VA_ARGS__)
#define paerr(fmt, ...)     apperr("[PipelineStaStart] " fmt, ##__VA_ARGS__)

/* Operation context */
typedef struct {
    wifi_connection_t *conn;
    void (*on_success)(void *ctx);
    void (*on_error)(void *ctx, int error);
    void *user_ctx;
    pipeline_t *pipeline;
} sta_connect_ctx_t;

static node_result_t check_node_if_need_wait(pipeline_node_t *node)
{
    if (!node_wait_event_is_empty(node)) {
        if (node->triggered_event_id == -1) {
            return NODE_WAITING;
        }
    }
    return NODE_SUCCESS;
}

/* Node Validate parameters */
static node_result_t node_validate(pipeline_node_t *node, int event_id, void *event_data)
{
    sta_connect_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    painfo("Node: %s\n", node->name);
    
    /* Check SSID */
    if (strlen(conn->ssid) == 0) {
        conn->error_code = WIFI_ERR_NO_SSID;
        paerr("No SSID\n");
        return NODE_FAILED;
    }
    
    /* Check password length if provided */
    if (strlen(conn->password) > 0 && strlen(conn->password) < 8) {
        conn->error_code = WIFI_ERR_SHORT_PASSWORD;
        paerr("Password too short\n");
        return NODE_FAILED;
    }
    
    /* Check current state */
    if (conn->state != WIFI_STATE_IDLE) {
        conn->error_code = WIFI_ERR_WRONG_STATE;
        paerr("Wrong state %d\n", conn->state);
        return NODE_FAILED;
    }
    
    conn->state = WIFI_STA_STATE_CONNECTING;
    return NODE_SUCCESS;
}

/* Node Set WiFi mode to STA */
static node_result_t node_set_mode_sta(pipeline_node_t *node, int event_id, void *event_data)
{
    sta_connect_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    painfo("Node: %s\n", node->name);
    
    int ret = wf_set_mode(WIFI_STA_DEVNAME, 2); /* WAPI_MODE_MANAGED = 2 */
    if (ret < 0) {
        conn->error_code = WIFI_ERR_SET_MODE_FAILED;
        paerr("Failed to set STA mode (%d)\n", ret);
        return NODE_FAILED;
    }
    
    conn->state = WIFI_STA_STATE_CONNECTING;
    return NODE_SUCCESS;
}

/* Node Bring up interface */
static node_result_t node_bringup(pipeline_node_t *node, int event_id, void *event_data)
{
    sta_connect_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    int ret = 0;

    painfo("Node: %s\n", node->name);
    
    if (event_id == NO_WAIT_EVENT) {
        /* Create socket for ioctl */
        int sock = wapi_make_socket();
        if (sock < 0) {
            paerr("Failed to create socket: %d\n", errno);
            conn->error_code = WIFI_ERR_SET_MODE_FAILED;
            return NODE_FAILED;
        } else {
            ret = wapi_set_ifup(sock, WIFI_STA_DEVNAME);
            if (ret < 0) {
                conn->error_code = WIFI_ERR_SET_MODE_FAILED;
                close(sock);
                return NODE_FAILED;
            }
            close(sock);
        }
    } else {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            padbg("event_id:%d WIFI_EVENT_STA_START\n", event_id);
            break;
        default:
            node->triggered_event_id = -1;
            break;
        }
    }

    return check_node_if_need_wait(node);
}

/* Node Set SSID */
static node_result_t node_set_ssid(pipeline_node_t *node, int event_id, void *event_data)
{
    sta_connect_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    int ret = 0;

    painfo("Node: %s\n", node->name);

    if (event_id == NO_WAIT_EVENT) {
        /* Create socket for ioctl */
        int sock = wapi_make_socket();
        if (sock < 0) {
            paerr("Failed to create socket: %d\n", errno);
            return;
        }

        /* Set SSID */
        ret = wapi_set_essid(sock, WIFI_STA_DEVNAME, conn->ssid, WAPI_ESSID_ON);
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_SSID_FAILED;
            paerr("Failed to set SSID (%d)\n", ret);
            return NODE_FAILED;
        }

        close(sock);
    } else {
        switch (event_id) {
        case WIFI_EVENT_STA_CONNECTED:
            padbg("event_id:%d WIFI_EVENT_STA_CONNECTED\n", event_id);
            break;
        default:
            node->triggered_event_id = -1;
            break;
        }
    }
    
    return check_node_if_need_wait(node);
}

/* Node Set password */
static node_result_t node_set_password(pipeline_node_t *node, int event_id, void *event_data)
{
    sta_connect_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;

    int ret = 0;

    painfo("Node: %s\n", node->name);
    
    /* Skip if no password */
    if (strlen(conn->password) == 0) {
        return NODE_SUCCESS;
    }

    if (event_id == NO_WAIT_EVENT) {
        ret = wf_set_psk(WIFI_STA_DEVNAME, conn->password);
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_PSK_FAILED;
            paerr("Failed to set password (%d)\n", ret);
            return NODE_FAILED;
        }
    }
    
    return check_node_if_need_wait(node);
}

/* Node Wait for connection event */
static node_result_t node_wait_connect(pipeline_node_t *node, int event_id, void *event_data)
{
    sta_connect_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    painfo("Node: %s\n", node->name);
    
    /* Subscribe to events if first time */
    if (event_id == -1) {
        painfo("Waiting for STA connection event...\n");
        /* Subscribe to WIFI_ADPT_EVT_STA_CONNECT event */
        return NODE_WAITING;
    }

    /* Check for connection success event (WIFI_ADPT_EVT_STA_CONNECT = 2) */
    if (event_id == WIFI_EVENT_STA_CONNECTED) {
        conn->state = WIFI_STA_STATE_CONNECTED;
        time(&conn->connect_start_time);
        painfo("STA connected!\n");
        return NODE_SUCCESS;
    } else {
        paerr("Error: STA connection failed\n");
        return NODE_FAILED;
    }
}

/* Node renew wlanx */
static node_result_t node_renewd(pipeline_node_t *node, int event_id, void *event_data)
{
    (void)event_id;
    (void)event_data;

    char cmd[100];

    painfo("Node: %s\n", node->name);

    snprintf(cmd, sizeof(cmd), "renew %s", WIFI_STA_DEVNAME);
    (void)system(cmd);

    return NODE_SUCCESS;
}

/* Pipeline completion callback */
static void pipeline_sta_complete(pipeline_t *p, bool success, void *user_data)
{
    sta_connect_ctx_t *ctx = (sta_connect_ctx_t *)user_data;
    wifi_connection_t *conn = ctx->conn;
    
    conn->active_pipeline = NULL;
    
    if (success) {
        painfo("STA Connect SUCCESS\n");
        if (conn->on_success) {
            conn->on_success(conn->user_ctx);
        }
    } else {
        paerr("STA Connect FAILED: %d\n", conn->error_code);
        if (conn->on_error) {
            conn->on_error(conn->user_ctx, conn->error_code);
        }
    }
    
    free(ctx);
    p->user_data = NULL;
    pipeline_destroy_async(p);
}

/* Create STA connect pipeline */
pipeline_t *pipeline_sta_connect_create(wifi_connection_t *conn,
                                        void (*on_complete)(pipeline_t *, bool, void *),
                                        void *user_ctx)
{
    sta_connect_ctx_t *ctx = (sta_connect_ctx_t *)calloc(1, sizeof(sta_connect_ctx_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->conn = conn;
    ctx->user_ctx = user_ctx;
    
    /* Extract callbacks from user_ctx or use on_complete directly */
    ctx->on_success = NULL;  /* Will be set by caller */
    ctx->on_error = NULL;
    
    pipeline_t *p = pipeline_create("sta_connect", 30000,  /* 30s timeout */
                                     pipeline_sta_complete, ctx);
    if (!p) {
        free(ctx);
        return NULL;
    }
    
    ctx->pipeline = p;
    
    /* Add nodes
     * wait_event_id: -1 表示不等待事件，立即执行
     *                其他值表示等待对应的 WiFi 事件
     */
    pipeline_add_node(p, "validate", node_validate, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "set_mode_sta", node_set_mode_sta, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "bringup", node_bringup, ctx, WIFI_EVENT_STA_START);
    pipeline_add_node(p, "set_password", node_set_password, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "set_ssid", node_set_ssid, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "wait_connect", node_wait_connect, ctx, WIFI_EVENT_STA_CONNECTED);
    pipeline_add_node(p, "renewd", node_renewd, ctx, NO_WAIT_EVENT);

    return p;
}
