/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi_manager/wifi_state.c
 *
 * WiFi Connection State Management Implementation
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "wifi_state.h"

wifi_connection_t *wifi_connection_create(const char *ssid, const char *password,
                                          bool is_ap)
{
    wifi_connection_t *conn = (wifi_connection_t *)calloc(1, sizeof(wifi_connection_t));
    if (!conn) {
        return NULL;
    }

    if (ssid) {
        strncpy(conn->ssid, ssid, sizeof(conn->ssid) - 1);
    }
    if (password) {
        strncpy(conn->password, password, sizeof(conn->password) - 1);
    }

    conn->is_ap_mode = is_ap;
    conn->state = WIFI_STATE_IDLE;
    conn->error_code = WIFI_ERR_NONE;
    conn->active_pipeline = NULL;
    conn->connect_start_time = 0;
    conn->connect_duration = 0;

    return conn;
}

void wifi_connection_destroy(wifi_connection_t *conn)
{
    if (conn) {
        if (conn->active_pipeline) {
            /* Pipeline should be destroyed by its own completion callback */
            conn->active_pipeline = NULL;
        }
        free(conn);
    }
}

void wifi_connection_set_state(wifi_connection_t *conn, wifi_conn_state_t state)
{
    if (conn) {
        conn->state = state;
        if ((state == WIFI_STA_STATE_CONNECTED) || (state == WIFI_AP_STATE_STARTED)) {
            time(&conn->connect_start_time);
        }
    }
}

void wifi_connection_set_error(wifi_connection_t *conn, int error_code)
{
    if (conn) {
        conn->error_code = error_code;
        conn->state = WIFI_STATE_MAX;
    }
}

const char *wifi_state_to_string(wifi_conn_state_t state)
{
    switch (state) {
        case WIFI_STATE_IDLE:               return "IDLE";
        case WIFI_STA_STATE_CONNECTING:     return "STA_CONNECTING";
        case WIFI_STA_STATE_CONNECTED:      return "STA_CONNECTED";
        case WIFI_STA_STATE_DISCONNECTED:   return "STA_DISCONNECTED";
        case WIFI_AP_STATE_STARTING:        return "AP_STARTING";
        case WIFI_AP_STATE_STARTED:         return "AP_STARTED";
        case WIFI_AP_STATE_STOPPED:         return "AP_STOPPED";
        case WIFI_STATE_ERROR:              return "ERROR";
        default:                            return "UNKNOWN";
    }
}
