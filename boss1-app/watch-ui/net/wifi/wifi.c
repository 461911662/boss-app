/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/wifi/wifi.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netutils/netlib.h>
#include <nuttx/wireless/wireless.h>
#include <wireless/wapi.h>

#ifdef CONFIG_NETUTILS_DHCPD
#  include <netutils/dhcpd.h>
#endif

#ifdef CONFIG_NETUTILS_DHCPC
#  include <netutils/dhcpc.h>
#endif

#include "watchui/log.h"

#include "wifi.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WIFI_STA_DEVNAME  "wlan0"
#define WIFI_AP_DEVNAME   "wlan1"

#define WIFI_CONNECT_TIMEOUT_MS  30000

#define USE_WAPI_WPA_VER        WPA_VER_2
#define USE_WAPI_WPA_CIPHER      WPA_ALG_CCMP

/****************************************************************************
 * Logging Macros
 ****************************************************************************/
#define WF_LOG(fmt, ...)    appinfo("[PwmWorker] " fmt, ##__VA_ARGS__)
#define WF_DBG(fmt, ...)   appdbg("[PwmWorker] " fmt, ##__VA_ARGS__)
#define WF_WARN(fmt, ...)  appwarn("[PwmWorker] " fmt, ##__VA_ARGS__)
#define WF_ERR(fmt, ...)   apperr("[PwmWorker] " fmt, ##__VA_ARGS__)

/****************************************************************************
 * Private Data
 ****************************************************************************/

static bool g_wifi_sta_connected = false;
static bool g_wifi_ap_started = false;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: wf_set_ipaddr
 *
 * Description:
 *   Set IP address for network interface.
 *
 ****************************************************************************/

static int wf_set_ipaddr(const char *ifname, in_addr_t ipaddr, in_addr_t netmask)
{
  int sock;
  int ret;

  sock = wapi_make_socket();
  if (sock < 0)
    {
      return sock;
    }

  ret = wapi_set_ip(sock, ifname, &ipaddr);
  if (ret < 0)
    {
      printf("ERROR: wapi_set_ip failed: %d\n", ret);
      close(sock);
      return ret;
    }

  ret = wapi_set_netmask(sock, ifname, &netmask);
  if (ret < 0)
    {
      printf("ERROR: wapi_set_netmask failed: %d\n", ret);
    }

  close(sock);
  return ret;
}

/****************************************************************************
 * Name: wf_bring_up
 *
 * Description:
 *   Bring up network interface.
 *
 ****************************************************************************/

static int wf_bring_up(const char *ifname)
{
  int sock;
  int ret;

  sock = wapi_make_socket();
  if (sock < 0)
    {
      return sock;
    }

  ret = wapi_set_ifup(sock, ifname);
  if (ret < 0)
    {
      printf("ERROR: wapi_set_ifup failed: %d\n", ret);
    }

  close(sock);
  return ret;
}

/****************************************************************************
 * Name: wf_bring_down
 *
 * Description:
 *   Bring down network interface.
 *
 ****************************************************************************/

static int wf_bring_down(const char *ifname)
{
  int sock;
  int ret;

  sock = wapi_make_socket();
  if (sock < 0)
    {
      return sock;
    }

  ret = wapi_set_ifdown(sock, ifname);
  if (ret < 0)
    {
      printf("ERROR: wapi_set_ifdown failed: %d\n", ret);
    }

  close(sock);
  return ret;
}

/****************************************************************************
 * Name: wf_disconnect
 *
 * Description:
 *   Disconnect WiFi connection.
 *
 ****************************************************************************/

static int wf_disconnect(const char *ifname)
{
  int sock;
  int ret;

  sock = wapi_make_socket();
  if (sock < 0)
    {
      return sock;
    }

  ret = wapi_disconnect(sock, ifname);
  if (ret < 0)
    {
      printf("ERROR: wapi_disconnect failed: %d\n", ret);
    }

  close(sock);
  return ret;
}

/****************************************************************************
 * Name: wf_set_mode
 *
 * Description:
 *   Set WiFi mode (STA or AP).
 *
 ****************************************************************************/

int wf_set_mode(const char *ifname, enum wapi_mode_e mode)
{
  int sock;
  int ret;

  sock = wapi_make_socket();
  if (sock < 0)
    {
      WF_ERR("wapi_make_socket failed: %d\n", sock);
      return sock;
    }

  ret = wapi_set_mode(sock, ifname, mode);
  if (ret < 0)
    {
      WF_ERR("wapi_set_mode failed: %d\n", ret);
    }

  close(sock);
  return ret;
}

/****************************************************************************
 * Name: wf_set_ssid
 *
 * Description:
 *   Set WiFi SSID.
 *
 ****************************************************************************/

static int wf_set_ssid(const char *ifname, const char *ssid)
{
  int sock;
  int ret;

  sock = wapi_make_socket();
  if (sock < 0)
    {
      return sock;
    }

  // ret = wapi_set_essid(sock, ifname, ssid, WAPI_ESSID_ON);
  ret = wapi_set_essid(sock, ifname, ssid, WAPI_ESSID_DELAY_ON);
  if (ret < 0)
    {
      printf("ERROR: wapi_set_essid failed: %d\n", ret);
    }

  close(sock);
  return ret;
}

/****************************************************************************
 * Name: wf_set_psk
 *
 * Description:
 *   Set WiFi password (PSK).
 *
 ****************************************************************************/

int wf_set_psk(const char* ifname, const char *psk)
{
#ifdef CONFIG_WIRELESS_WAPI
  enum wpa_alg_e alg_flag;
  enum wpa_ver_e ver_flag;
  uint8_t auth_wpa;
  int passlen;
  int cipher;
  int ret;

  int sock = wapi_make_socket();
  if (sock < 0)
    {
      return sock;
    }

  /* Check if password len >= 8 && <= 63 */

  passlen = strnlen(psk, 64);
  if (passlen < 8 || passlen > 63)
    {
      printf("The password should have between 8 and 63 characters!\n");
      return -EINVAL;
    }

  /* Convert input strings to values */

  ver_flag = USE_WAPI_WPA_VER;

  switch (ver_flag)
    {
      case WPA_VER_NONE:
        auth_wpa = IW_AUTH_WPA_VERSION_DISABLED;
        break;

      case WPA_VER_1:
        auth_wpa = IW_AUTH_WPA_VERSION_WPA;
        break;

      case WPA_VER_2:
        auth_wpa = IW_AUTH_WPA_VERSION_WPA2;
        break;

      case WPA_VER_3:
        auth_wpa = IW_AUTH_WPA_VERSION_WPA3;
        break;

      default:
        return -EINVAL;
    }

  alg_flag = USE_WAPI_WPA_CIPHER;

  switch (alg_flag)
    {
      case WPA_ALG_NONE:
        cipher = IW_AUTH_CIPHER_NONE;
        break;

      case WPA_ALG_WEP:
        cipher = IW_AUTH_CIPHER_WEP40;
        break;

      case WPA_ALG_TKIP:
        cipher = IW_AUTH_CIPHER_TKIP;
        break;

      case WPA_ALG_CCMP:
        cipher = IW_AUTH_CIPHER_CCMP;
        break;

      default:
        return -EINVAL;
    }

  ret = wpa_driver_wext_set_auth_param(sock, ifname,
                                       IW_AUTH_WPA_VERSION,
                                       auth_wpa);
  if (ret >= 0)
    {
      ret = wpa_driver_wext_set_auth_param(sock, ifname,
                                           IW_AUTH_CIPHER_PAIRWISE,
                                           cipher);

      /* Set the Passphrase */

      if (ret >= 0)
        {
          ret = wpa_driver_wext_set_key_ext(sock, ifname, alg_flag,
                                            psk, passlen);
        }
    }

  close(sock);

  return ret;
#else
  return OK;
#endif
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_WIRELESS_WAPI
/****************************************************************************
 * Name: wifi_sta_connect
 ****************************************************************************/

int wf_sta_connect(FAR wifi_sta_config_t *config,
                     FAR wifi_sta_services_t *services)
{
  int ret;

  if (config == NULL)
    {
      return -EINVAL;
    }

  printf("Connecting to WiFi SSID: %s\n", config->ssid);

  ret = wf_set_mode(WIFI_STA_DEVNAME, WAPI_MODE_MANAGED);
  if (ret < 0)
    {
      return ret;
    }

  ret = wf_set_ssid(WIFI_STA_DEVNAME, config->ssid);
  if (ret < 0)
    {
      return ret;
    }

  // if (config->password[0] != '\0')
  //   {
  //     ret = wf_set_psk(WIFI_STA_DEVNAME, config->password);
  //     if (ret < 0)
  //       {
  //         return ret;
  //       }
  //   }

  ret = wf_bring_up(WIFI_STA_DEVNAME);
  if (ret < 0)
    {
      return ret;
    }

  g_wifi_sta_connected = true;
  printf("WiFi STA started\n");

  return OK;
}

/****************************************************************************
 * Name: wf_sta_disconnect
 ****************************************************************************/

int wf_sta_disconnect(void)
{
  int ret;

  ret = wf_disconnect(WIFI_STA_DEVNAME);
  if (ret < 0)
    {
      printf("WARNING: wifi_disconnect failed: %d\n", ret);
    }

  ret = wf_bring_down(WIFI_STA_DEVNAME);
  if (ret < 0)
    {
      return ret;
    }

  g_wifi_sta_connected = false;
  printf("WiFi STA stopped\n");

  return OK;
}

/****************************************************************************
 * Name: wifi_sta_is_connected
 ****************************************************************************/

bool wf_sta_is_connected(void)
{
  return g_wifi_sta_connected;
}

/****************************************************************************
 * Name: wifi_ap_start
 ****************************************************************************/

int wf_ap_start(FAR wifi_ap_config_t *config,
                  FAR wifi_ap_dhcp_t *dhcp,
                  FAR wifi_ap_services_t *services)
{
  int ret;
  struct in_addr addr;
  FAR struct wpa_wconfig_s *wconfig = NULL;

  if (config == NULL)
    {
      return -EINVAL;
    }

  WF_DBG("Starting WiFi AP\n");

  addr.s_addr = config->ipaddr;
  netlib_set_ipv4addr(WIFI_AP_DEVNAME, &addr);

  addr.s_addr = config->netmask;
  netlib_set_ipv4netmask(WIFI_AP_DEVNAME, &addr);

  netlib_ifup(WIFI_AP_DEVNAME);

  sleep(1);

  ret = wf_set_ssid(WIFI_AP_DEVNAME, config->ssid);
  if (ret < 0)
    {
      WF_WARN("wapi_set_essid failed: %d\n", ret);
    }

  sleep(2);

  if (config->password[0] != '\0')
    {
      ret = wf_set_psk(WIFI_AP_DEVNAME, config->password);
      if (ret < 0)
        {
          WF_WARN("wapi_set_psk failed: %d\n", ret);
          return ret;
        }
    }

  // wconfig = (struct wpa_wconfig_s *)malloc(sizeof(struct wpa_wconfig_s));

  // if (wconfig == NULL)
  // {
  //   WF_ERR("Failed to allocate memory for wpa_wconfig_s\n");
  //   return -ENOMEM;
  // }

  // memset(wconfig, 0, sizeof(struct wpa_wconfig_s));
  // wconfig->ifname = WIFI_AP_DEVNAME;
  // wconfig->sta_mode = WAPI_MODE_MASTER;
  // wconfig->ssid = config->ssid;
  // wconfig->ssidlen = strlen(wconfig->ssid);

  // wconfig->passphrase = config->password;
  // wconfig->phraselen = strlen(config->password);

  // // 6. 设置 WPA2-PSK 加密
  // wconfig->auth_wpa = IW_AUTH_WPA_VERSION_WPA2;
  // wconfig->cipher_mode = IW_AUTH_CIPHER_CCMP;
  // wconfig->alg = WPA_ALG_CCMP;

  // wpa_driver_wext_associate(wconfig);

  // wconfig->auth_wpa = IW_AUTH_WPA_VERSION_DISABLED;
  // wconfig->cipher_mode = IW_AUTH_CIPHER_NONE;
  // wconfig->alg = WPA_ALG_NONE;


  // ret = wf_set_mode(WIFI_AP_DEVNAME, WAPI_MODE_MASTER);
  // if (ret < 0)
  //   {
  //     WF_WARN("wapi_set_mode failed: %d\n", ret);
  //   }

  // ret = wf_set_ssid(WIFI_AP_DEVNAME, config->ssid);
  // if (ret < 0)
  //   {
  //     WF_WARN("wapi_set_essid failed: %d\n", ret);
  //   }

  // if (config->password[0] != '\0')
  //   {
  //     ret = wf_set_psk(WIFI_AP_DEVNAME, config->password);
  //     if (ret < 0)
  //       {
  //         WF_WARN("wapi_set_psk failed: %d\n", ret);
  //         return ret;
  //       }
  //   }

  // ret = wf_bring_up(WIFI_AP_DEVNAME);
  // if (ret < 0)
  //   {
  //     WF_ERR("wapi_bring_up failed: %d\n", ret);
  //     return ret;
  //   }

  // ret = wf_set_ipaddr(WIFI_AP_DEVNAME, config->ipaddr, config->netmask);
  // if (ret < 0)
  //   {
  //     WF_ERR("wapi_set_ipaddr failed: %d\n", ret);
  //     return ret;
  //   }

#ifdef CONFIG_NETUTILS_DHCPD
  if (dhcp != NULL && dhcp->enable)
    {
      // ret = dhcpd_set_routerip(dhcp->dns_ip);
      // if (ret < 0)
      //   {
      //     printf("WARNING: dhcpd_set_routerip failed: %d\n", ret);
      //   }

      // ret = dhcpd_set_netmask(dhcp->start_ip);
      // if (ret < 0)
      //   {
      //     printf("WARNING: dhcpd_set_netmask failed: %d\n", ret);
      //   }

      // ret = dhcpd_set_dnsip(dhcp->dns_ip);
      // if (ret < 0)
      //   {
      //     printf("WARNING: dhcpd_set_dnsip failed: %d\n", ret);
      //   }

      ret = dhcpd_start(WIFI_AP_DEVNAME);
      if (ret < 0)
        {
          printf("ERROR: dhcpd_start failed: %d\n", ret);
          return ret;
        }

      printf("DHCP Server started\n");
    }
#endif

  // ret = wpa_driver_wext_associate(wconfig);
  // if (ret < 0)
  // {
  //   WF_WARN("failed: %d\n", ret);
  // }

  // ret = wf_bring_up(WIFI_AP_DEVNAME);
  // if (ret < 0)
  //   {
  //     WF_ERR("wapi_bring_up failed: %d\n", ret);
  //     return ret;
  //   }

  g_wifi_ap_started = true;
  printf("WiFi AP started\n");
  // free(wconfig);

  return OK;
}

int wf_ap_dhcpd_start(void)
{
    int ret = dhcpd_start(WIFI_AP_DEVNAME);
    if (ret < 0)
      {
        printf("ERROR: dhcpd_start failed: %d\n", ret);
        return ret;
      }

    printf("DHCP Server started\n");
    return ret;
}

/****************************************************************************
 * Name: wifi_ap_stop
 ****************************************************************************/

int wf_ap_stop(void)
{
  int ret;

#ifdef CONFIG_NETUTILS_DHCPD
  ret = dhcpd_stop();
  if (ret < 0)
    {
      printf("WARNING: dhcpd_stop failed: %d\n", ret);
    }
#endif

  ret = wf_bring_down(WIFI_AP_DEVNAME);
  if (ret < 0)
    {
      return ret;
    }

  g_wifi_ap_started = false;
  printf("WiFi AP stopped\n");

  return OK;
}

#endif /* CONFIG_WIRELESS_WAPI */
