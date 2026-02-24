/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi/wifi.h
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

#ifndef __VENDOR_BOSS_APP_BOSS1_APP_WATCH_UI_NET_WIFI_H
#define __VENDOR_BOSS_APP_BOSS1_APP_WATCH_UI_NET_WIFI_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <wireless/wapi.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef __ASSEMBLY__

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#define WIFI_STA_DEVNAME  "wlan0"
#define WIFI_AP_DEVNAME   "wlan1"

#define WIFI_MAX_SSID_LEN       32
#define WIFI_MAX_PASSWORD_LEN  64


/****************************************************************************
 * STA Types
 ****************************************************************************/

typedef struct {
    char      ssid[WIFI_MAX_SSID_LEN + 1];
    char      password[WIFI_MAX_PASSWORD_LEN + 1];
    uint8_t   bssid[6];
    bool      bssid_set;
} wifi_sta_config_t;

typedef struct {
    bool      dhcp_enable;
    bool      ntp_enable;
} wifi_sta_services_t;

/****************************************************************************
 * AP Types
 ****************************************************************************/

typedef struct {
    char      ssid[WIFI_MAX_SSID_LEN + 1];
    char      password[WIFI_MAX_PASSWORD_LEN + 1];
    bool      ssid_hidden;
    uint8_t   max_connections;
    uint16_t  beacon_interval;
    in_addr_t ipaddr;
    in_addr_t netmask;
} wifi_ap_config_t;

typedef struct {
    bool      enable;
    in_addr_t start_ip;
    in_addr_t end_ip;
    in_addr_t dns_ip;
} wifi_ap_dhcp_t;

typedef struct {
    bool      dhcp_enable;
    bool      web_enable;
    uint16_t  web_port;
} wifi_ap_services_t;

/****************************************************************************
 * STA API
 ****************************************************************************/

#ifdef CONFIG_WIRELESS_WAPI
/****************************************************************************
 * Name: wifi_sta_connect
 *
 * Description:
 *   Connect to WiFi network as Station.
 *
 * Input Parameters:
 *   config - STA configuration (ssid, password, etc.)
 *   services - Services to start (dhcp, ntp)
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned
 *   to indicate the nature of any failure.
 *
 ****************************************************************************/

int wf_sta_connect(FAR wifi_sta_config_t *config,
                     FAR wifi_sta_services_t *services);

/****************************************************************************
 * Name: wf_sta_disconnect
 *
 * Description:
 *   Disconnect from WiFi network.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned
 *   to indicate the nature of any failure.
 *
 ****************************************************************************/

int wf_sta_disconnect(void);

/****************************************************************************
 * Name: wf_sta_is_connected
 *
 * Description:
 *   Check if STA is connected to WiFi network.
 *
 * Returned Value:
 *   true if connected, false otherwise.
 *
 ****************************************************************************/

bool wf_sta_is_connected(void);

/****************************************************************************
 * Name: wf_ap_start
 *
 * Description:
 *   Start WiFi Access Point.
 *
 * Input Parameters:
 *   config - AP configuration (ssid, password, ip, etc.)
 *   dhcp - DHCP server configuration
 *   services - Services to start (dhcp, web)
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned
 *   to indicate the nature of any failure.
 *
 ****************************************************************************/

int wf_ap_start(FAR wifi_ap_config_t *config,
                  FAR wifi_ap_dhcp_t *dhcp,
                  FAR wifi_ap_services_t *services);

/****************************************************************************
 * Name: wf_ap_stop
 *
 * Description:
 *   Stop WiFi Access Point.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returned
 *   to indicate the nature of any failure.
 *
 ****************************************************************************/

int wf_ap_stop(void);

int wf_set_mode(const char *ifname, enum wapi_mode_e mode);

int wf_set_psk(const char* ifname, const char *psk);

int wf_ap_dhcpd_start(void);

#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __VENDOR_BOSS_APP_BOSS1_APP_WATCH_UI_NET_WIFI_H */

