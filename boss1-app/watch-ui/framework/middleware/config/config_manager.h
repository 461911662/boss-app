/****************************************************************************
 * watch-ui/framework/middleware/config/config_manager.h
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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#ifndef __WATCH_UI_FRAMEWORK_MIDDLEWARE_CONFIG_CONFIG_MANAGER_H
#define __WATCH_UI_FRAMEWORK_MIDDLEWARE_CONFIG_CONFIG_MANAGER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CONFIG_WIFI_SSID_MAX_LEN       32
#define CONFIG_WIFI_PASSWORD_MAX_LEN    64
#define CONFIG_USERNAME_MAX_LEN         32
#define CONFIG_JSON_DEFAULT_SIZE        2048

typedef enum
{
  ACCOUNT_STATUS_INVALID = -1,
  ACCOUNT_OFFLINE = 0,
  ACCOUNT_ONLINE  = 1
} account_status_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: config_load
 *
 * Description:
 *   Load configuration from file.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   Returns a pointer to the cJSON object on success;
 *   NULL on failure.
 *
 ****************************************************************************/

FAR void *config_load(void);

/****************************************************************************
 * Name: config_save
 *
 * Description:
 *   Save configuration to file.
 *
 * Input Parameters:
 *   json_obj - cJSON object pointer to save
 *
 * Returned Value:
 *   Zero (OK) on success; negative error code on failure.
 *
 ****************************************************************************/

int config_save(FAR void *json_obj);

/****************************************************************************
 * Name: config_free
 * Description:
 *   Free cJSON object.
 * Input Parameters:
 *   json_obj - cJSON object pointer to free
 * 
 * Returned Value:
 *   None
 * ****************************************************************************/

void config_free(FAR void *json_obj);

/****************************************************************************
 * Name: config_get_wifi_sta
 *
 * Description:
 *   Get WiFi STA configuration.
 *
 * Input Parameters:
 *   ssid     - Buffer to store SSID
 *   password - Buffer to store password
 *   maxlen   - Maximum length of buffers
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_get_wifi_sta(FAR char *ssid, FAR char *password, size_t maxlen);

/****************************************************************************
 * Name: config_set_wifi_sta
 *
 * Description:
 *   Set WiFi STA configuration.
 *
 * Input Parameters:
 *   ssid     - SSID string
 *   password - Password string
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_set_wifi_sta(FAR const char *ssid, FAR const char *password);

/****************************************************************************
 * Name: config_get_wifi_ap_ssid
 *
 * Description:
 *   Get WiFi AP SSID.
 *
 * Input Parameters:
 *   ssid   - Buffer to store SSID
 *   maxlen - Maximum length of buffer
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_get_wifi_ap_ssid(FAR char *ssid, size_t maxlen);

/****************************************************************************
 * Name: config_set_wifi_ap_ssid
 *
 * Description:
 *   Set WiFi AP SSID.
 *
 * Input Parameters:
 *   ssid - SSID string
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_set_wifi_ap_ssid(FAR const char *ssid);

/****************************************************************************
 * Name: config_get_max_users
 *
 * Description:
 *   Get max users limit.
 *
 * Returned Value:
 *   Max users count, or negative error code on failure.
 *
 ****************************************************************************/

int config_get_max_users(void);

/****************************************************************************
 * Name: config_get_userinfo
 *
 * Description:
 *   Get user security configuration (lockout_threshold,
 *   lockout_duration, session_timeout).
 *
 * Input Parameters:
 *   threshold - Pointer to store lockout threshold
 *   duration  - Pointer to store lockout duration
 *   timeout   - Pointer to store session timeout
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_get_userinfo(FAR int *threshold, FAR int *duration,
                      FAR int *timeout);

/****************************************************************************
 * Name: config_set_userinfo
 *
 * Description:
 *   Set user security configuration (lockout_threshold,
 *   lockout_duration, session_timeout).
 *
 * Input Parameters:
 *   threshold - Lockout threshold value to set (-1 to keep current)
 *   duration  - Lockout duration value to set (-1 to keep current)
 *   timeout   - Session timeout value to set (-1 to keep current)
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_set_userinfo(int threshold, int duration, int timeout);

/****************************************************************************
 * Name: config_update_account_state
 *
 * Description:
 *   Update account state (online/offline).
 *
 * Input Parameters:
 *   username - Username
 *   status   - Account status
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   failure.
 *
 ****************************************************************************/

int config_update_account_state(FAR const char *username, account_status_t status);

/****************************************************************************
 * Name: config_get_account_state
 *
 * Description:
 *   Get account state.
 *
 * Input Parameters:
 *   username - Username
 *
 * Returned Value:
 *   Account status (ACCOUNT_ONLINE/ACCOUNT_OFFLINE), ACCOUNT_STATUS_INVALID on failure.
 *
 ****************************************************************************/

account_status_t config_get_account_state(FAR const char *username);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_UI_FRAMEWORK_MIDDLEWARE_CONFIG_CONFIG_MANAGER_H */
