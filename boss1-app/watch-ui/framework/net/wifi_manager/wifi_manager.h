/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi_manager/wifi_manager.h
 *
 * WiFi Manager with Pipeline-based Operations
 *
 ****************************************************************************/

#ifndef __WATCH_UI_NET_WIFI_MANAGER_H
#define __WATCH_UI_NET_WIFI_MANAGER_H

#include <nuttx/config.h>
#include <uv.h>

#include "wifi_state.h"
#include "pipeline/pipeline.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ESP IDF WIFI EVENT */
#define WIFI_EVENT_SCAN_DONE            1     /* -> WIFI_ADPT_EVT_SCAN_DONE            0 */
#define WIFI_EVENT_STA_START            2     /* -> WIFI_ADPT_EVT_STA_START            1 */
#define WIFI_EVENT_STA_STOP             3     /* -> WIFI_ADPT_EVT_STA_STOP             5 */
#define WIFI_EVENT_STA_CONNECTED        4     /* -> WIFI_ADPT_EVT_STA_CONNECT          2 */
#define WIFI_EVENT_STA_DISCONNECTED     5     /* -> WIFI_ADPT_EVT_STA_DISCONNECT       3 */
#define WIFI_EVENT_STA_AUTHMODE_CHANGE  6     /* -> WIFI_ADPT_EVT_STA_AUTHMODE_CHANGE  4 */

#define WIFI_EVENT_AP_START             12    /* -> WIFI_ADPT_EVT_AP_START             6 */
#define WIFI_EVENT_AP_STOP              13    /* -> WIFI_ADPT_EVT_AP_STOP              7 */
#define WIFI_EVENT_AP_STACONNECTED      14    /* -> WIFI_ADPT_EVT_AP_STACONNECTED      8 */
#define WIFI_EVENT_AP_STADISCONNECTED   15    /* -> WIFI_ADPT_EVT_AP_STADISCONNECTED   9 */

/* WiFi Manager structure */
typedef struct {
    uv_loop_t *loop;
    uv_async_t event_async;        /* WiFi event notification */

    /* Connections */
    wifi_connection_t *sta_conn;   /* STA connection */
    wifi_connection_t *ap_conn;    /* AP connection */
} wifi_manager_t;

/* Manager lifecycle */
int wifi_manager_init(uv_loop_t *loop);
wifi_manager_t *wifi_manager_get(void);
void wifi_manager_deinit(void);

/* High-level WiFi operations with Pipeline */
int wifi_manager_connect(const char *ssid, const char *password,
                         void (*on_success)(void *ctx),
                         void (*on_error)(void *ctx, int error),
                         void *user_ctx);

int wifi_manager_disconnect(void (*on_success)(void *ctx),
                            void (*on_error)(void *ctx, int error),
                            void *user_ctx);

int wifi_manager_start_ap(const char *ssid, const char *password,
                          in_addr_t ipaddr, in_addr_t netmask,
                          void (*on_success)(void *ctx),
                          void (*on_error)(void *ctx, int error),
                          void *user_ctx);

int wifi_manager_stop_ap(void (*on_success)(void *ctx),
                         void (*on_error)(void *ctx, int error),
                         void *user_ctx);

/* Internal: Handle WiFi events from adapter */
void wifi_manager_handle_event(int event_id, void *event_data);

/* Pipeline creation functions */
pipeline_t *pipeline_sta_connect_create(wifi_connection_t *conn,
                                        void (*on_complete)(pipeline_t *, bool, void *),
                                        void *user_ctx);

pipeline_t *pipeline_sta_disconnect_create(wifi_connection_t *conn,
                                           void (*on_complete)(pipeline_t *, bool, void *),
                                           void *user_ctx);

pipeline_t *pipeline_ap_start_create(wifi_connection_t *conn, void *user_ctx);

pipeline_t *pipeline_ap_stop_create(wifi_connection_t *conn,
                                    void (*on_complete)(pipeline_t *, bool, void *),
                                    void *user_ctx);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_UI_NET_WIFI_MANAGER_H */
