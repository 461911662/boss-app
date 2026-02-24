/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi_manager/wifi_state.h
 *
 * WiFi Connection State Management
 *
 ****************************************************************************/

#ifndef __WATCH_UI_NET_WIFI_STATE_H
#define __WATCH_UI_NET_WIFI_STATE_H

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* WiFi connection state */
typedef enum {
    WIFI_STATE_IDLE = 0,
    /* STA */
    WIFI_STA_STATE_CONNECTING,
    WIFI_STA_STATE_CONNECTED,
    WIFI_STA_STATE_DISCONNECTED,
    /* AP */
    WIFI_AP_STATE_STARTING,
    WIFI_AP_STATE_STARTED,
    WIFI_AP_STATE_STOPPED,
    WIFI_STATE_ERROR,
    WIFI_STATE_MAX,
} wifi_conn_state_t;

/* Error codes */
#define WIFI_ERR_NONE                   0
#define WIFI_ERR_NO_SSID               -1
#define WIFI_ERR_SHORT_PASSWORD        -2
#define WIFI_ERR_WRONG_STATE           -3
#define WIFI_ERR_SET_MODE_FAILED       -4
#define WIFI_ERR_SET_SSID_FAILED       -5
#define WIFI_ERR_SET_PSK_FAILED        -6
#define WIFI_ERR_BRINGUP_FAILED        -7
#define WIFI_ERR_CONNECT_FAILED        -8
#define WIFI_ERR_DHCP_FAILED           -9
#define WIFI_ERR_INVALID_CHANNEL       -10
#define WIFI_ERR_DHCPD_FAILED          -11
#define WIFI_ERR_AP_START_FAILED       -12
#define WIFI_ERR_TIMEOUT               -13
#define WIFI_ERR_CANCELLED             -14
#define WIFI_ERR_SET_IP_FAILED         -15
#define WIFI_ERR_SET_IPNETMSK_FAILED   -15

/* Pipeline forward declaration */
struct pipeline;

/* WiFi connection structure */
typedef struct wifi_connection {
    /* Configuration */
    char ssid[33];
    char password[65];
    bool is_ap_mode;               /* true=AP, false=STA */
    
    /* State */
    wifi_conn_state_t state;
    int error_code;
    
    /* Connection info (STA mode) */
    uint8_t bssid[6];
    int rssi;
    uint8_t auth_mode;
    
    /* IP configuration */
    in_addr_t local_ip;
    in_addr_t netmask;
    in_addr_t gateway;
    in_addr_t dns;
    
    /* Statistics */
    time_t connect_start_time;
    uint32_t connect_duration;     /* seconds */
    
    /* Pipeline association */
    struct pipeline *active_pipeline;
    void *user_ctx;
    
    /* Callbacks */
    void (*on_success)(void *ctx);
    void (*on_error)(void *ctx, int error);
    
    /* Linked list for manager */
    struct wifi_connection *next;
} wifi_connection_t;

/* Connection creation/destruction */
wifi_connection_t *wifi_connection_create(const char *ssid, const char *password,
                                          bool is_ap);
void wifi_connection_destroy(wifi_connection_t *conn);

/* State management */
void wifi_connection_set_state(wifi_connection_t *conn, wifi_conn_state_t state);
void wifi_connection_set_error(wifi_connection_t *conn, int error_code);
const char *wifi_state_to_string(wifi_conn_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_UI_NET_WIFI_STATE_H */
