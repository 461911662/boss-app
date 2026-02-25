/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi_manager/pipeline_ap_start.c
 *
 * AP Start Pipeline Implementation (Simplified)
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "wifi_manager.h"
#include "wifi.h"
#include "pipeline.h"
#include "watchui/log.h"

/****************************************************************************
 * Logging Macros
 ****************************************************************************/
#define ppinfo(fmt, ...)   appinfo("[PipelineApStart] " fmt, ##__VA_ARGS__)
#define ppdbg(fmt, ...)    appdbg("[PipelineApStart] " fmt, ##__VA_ARGS__)
#define ppwarn(fmt, ...)   appwarn("[PipelineApStart] " fmt, ##__VA_ARGS__)
#define pperr(fmt, ...)    apperr("[PipelineApStart] " fmt, ##__VA_ARGS__)

/* Operation context */
typedef struct {
    wifi_connection_t *conn;
    void (*on_success)(void *ctx);
    void (*on_error)(void *ctx, int error);
    void *user_ctx;
} ap_start_ctx_t;

static node_result_t check_node_if_need_wait(pipeline_node_t *node)
{
    if (!node_wait_event_is_empty(node)) {
        if (node->triggered_event_id == -1) {
            return NODE_WAITING;
        }
    }
    return NODE_SUCCESS;
}

/* Node 1: Validate AP parameters */
static node_result_t node_ap_validate(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    ppdbg("Node: %s\n", node->name);
    
    if (strlen(conn->ssid) == 0) {
        conn->error_code = WIFI_ERR_NO_SSID;
        return NODE_FAILED;
    }

    conn->state = WIFI_AP_STATE_STARTING;
    return check_node_if_need_wait(node);
}

/* Node 2: Set AP mode */
static node_result_t node_ap_set_mode(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    ppdbg("Node: %s\n", node->name);
    if (event_id == NO_WAIT_EVENT) {
        int ret = wf_set_mode(WIFI_AP_DEVNAME, 3); /* WAPI_MODE_MASTER = 3 */
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_MODE_FAILED;
            return NODE_FAILED;
        }
    }
    
    return check_node_if_need_wait(node);
}

/* Node: BringUp AP interface */
static node_result_t node_ap_bringup(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;

    int sock;
    int ret;

    ppdbg("Node: %s\n", node->name);
    if (event_id == NO_WAIT_EVENT) {
        /* Create socket for ioctl */
        sock = wapi_make_socket();
        if (sock < 0) {
            pperr("Failed to create socket: %d\n", errno);
            conn->error_code = WIFI_ERR_SET_MODE_FAILED;
            return NODE_FAILED;
        } else {
            ret = wapi_set_ifup(sock, WIFI_AP_DEVNAME);
            if (ret < 0) {
                conn->error_code = WIFI_ERR_SET_MODE_FAILED;
                close(sock);
                return NODE_FAILED;
            }
            close(sock);
        }
    }

    return check_node_if_need_wait(node);
}

static node_result_t node_ap_dhcp_start(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;

    (void)event_id;
    (void)event_data;

    ppdbg("Node: %s\n", node->name);
    if (event_id == NO_WAIT_EVENT) {
        int ret = wf_ap_dhcpd_start();
        if (ret < 0) {
            conn->error_code = WIFI_ERR_DHCP_FAILED;
            return NODE_FAILED;
        }
    }

    return check_node_if_need_wait(node);
}

/* Node: Configure AP SSID */
static node_result_t node_ap_config_ssid(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    (void)event_id;
    (void)event_data;
    
    int ret;
    static int ap_start_cnt = 0;

    ppdbg("Node: %s\n", node->name);

    if (event_id == NO_WAIT_EVENT) {
        /* Create socket for ioctl */
        int sock = wapi_make_socket();
        if (sock < 0) {
            pperr("Failed to create socket: %d\n", errno);
            return NODE_FAILED;
        }

        /* Set SSID */
        ret = wapi_set_essid(sock, WIFI_AP_DEVNAME, conn->ssid, WAPI_ESSID_ON);
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_SSID_FAILED;
            return NODE_FAILED;
        }
        ap_start_cnt = 0;

        close(sock);
    } else {
        ppdbg("event_id: %d\n", event_id);
        switch (event_id) {
        case WIFI_EVENT_AP_START:
            ap_start_cnt++;
            if (ap_start_cnt >= 2) {
                conn->state = WIFI_AP_STATE_STARTED;
            } else {
                node->triggered_event_id = -1;
            }
            break;
        case WIFI_EVENT_AP_STOP:
        default:
            node->triggered_event_id = -1;
            break;
        }
    }

    return check_node_if_need_wait(node);
}

/* Node: Configure AP PSK */
static node_result_t node_ap_config_psk(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;
    
    int ret;
    
    (void)event_id;
    (void)event_data;
    
    ppdbg("Node: %s\n", node->name);

    /* Set password if provided */
    if (strlen(conn->password) > 0) {
        ret = wf_set_psk(WIFI_AP_DEVNAME, conn->password);
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_PSK_FAILED;
            return NODE_FAILED;
        }
    }

    return check_node_if_need_wait(node);
}

/* Node 4: Set IP address */
static node_result_t node_ap_set_ip(pipeline_node_t *node, int event_id, void *event_data)
{
    ap_start_ctx_t *op_ctx = node->ctx;
    wifi_connection_t *conn = op_ctx->conn;

    (void)event_id;
    (void)event_data;
    
    int ret = 0;
    ppdbg("Node: %s\n", node->name);

    if (event_id == NO_WAIT_EVENT) {
        /* Create socket for ioctl */
        int sock = wapi_make_socket();
        if (sock < 0) {
            pperr("Failed to create socket: %d\n", errno);
            return NODE_FAILED;
        }

        ret = wapi_set_ip(sock, WIFI_AP_DEVNAME, &conn->local_ip);
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_IP_FAILED;
            return NODE_FAILED;
        }

        ret = wapi_set_netmask(sock, WIFI_AP_DEVNAME, &conn->netmask);
        if (ret < 0) {
            conn->error_code = WIFI_ERR_SET_IPNETMSK_FAILED;
            return NODE_FAILED;
        }

        close(sock);
    } else {

    }
    
    return check_node_if_need_wait(node);
}

/* Pipeline completion callback */
static void pipeline_ap_start_complete(pipeline_t *p, bool success, void *user_data)
{
    ap_start_ctx_t *ctx = (ap_start_ctx_t *)user_data;
    wifi_connection_t *conn = ctx->conn;
    
    conn->active_pipeline = NULL;
    
    if (success) {
        ppinfo("SUCCESS\n");
        if (conn->on_success) {
            conn->on_success(conn->user_ctx);
        }
    } else {
        pperr("FAILED: %d\n", conn->error_code);
        if (conn->on_error) {
            conn->on_error(conn->user_ctx, conn->error_code);
        }
    }
    
    free(ctx);
    p->user_data = NULL;
    pipeline_destroy_async(p);  /* Use async destroy to avoid loop exit */
}

/* Create AP start pipeline */
pipeline_t *pipeline_ap_start_create(wifi_connection_t *conn, void *user_ctx)
{
    ap_start_ctx_t *ctx = (ap_start_ctx_t *)calloc(1, sizeof(ap_start_ctx_t));
    if (!ctx) {
        return NULL;
    }

    ctx->conn = conn;
    ctx->user_ctx = user_ctx;

    pipeline_t *p = pipeline_create("ap_start", 60000,  /* 10s timeout */
                                     pipeline_ap_start_complete, ctx);
    if (!p) {
        free(ctx);
        return NULL;
    }

    /* Add nodes
     * pipeline_add_node_v(p, name, func, ctx, wait_count, event1, event2, ...)
     * wait_count: 等待的事件数量，0 表示不等待立即执行
     */
    pipeline_add_node(p, "validate", node_ap_validate, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "set_ip", node_ap_set_ip, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "ap_dhcpd_start", node_ap_dhcp_start, ctx, NO_WAIT_EVENT);
    pipeline_add_node(p, "ap_bringup", node_ap_bringup, ctx, WIFI_EVENT_AP_START);
    pipeline_add_node(p, "config_ssid", node_ap_config_ssid, ctx, WIFI_EVENT_AP_START);
    // pipeline_add_node(p, "config_psk", node_ap_config_psk, ctx, NO_WAIT_EVENT);

    return p;
}
