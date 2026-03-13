/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/framework/middleware/config/config_manager.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <uv.h>

#include "config_manager.h"
#include "config_crypto.h"
#include <netutils/cJSON.h>
#include <watchui/log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_CONFIG_DEBUG
#  define CFG_LOG(fmt, ...)    appinfo("[Config] " fmt, ##__VA_ARGS__)
#  define CFG_DBG(fmt, ...)    appdbg("[Config] " fmt, ##__VA_ARGS__)
#  define CFG_WARN(fmt, ...)   appwarn("[Config] " fmt, ##__VA_ARGS__)
#  define CFG_ERR(fmt, ...)    apperr("[Config] " fmt, ##__VA_ARGS__)
#else
#  define CFG_LOG(fmt, ...)
#  define CFG_DBG(fmt, ...)
#  define CFG_WARN(fmt, ...)   appwarn("[Config] " fmt, ##__VA_ARGS__)
#  define CFG_ERR(fmt, ...)    apperr("[Config] " fmt, ##__VA_ARGS__)
#endif

#ifdef CONFIG_CONFIG_FILE_DEBUG
#  define CONFIG_FILE_PATH "/etc/config.json"
#else
#  define CONFIG_FILE_PATH "/etc/config.json.enc"
#endif

#define DEFAULT_MAX_USERS          10
#define DEFAULT_LOCKOUT_THRESHOLD  5
#define DEFAULT_LOCKOUT_DURATION   300
#define DEFAULT_SESSION_TIMEOUT    3600

#define DEFAULT_STA_SSID       ""
#define DEFAULT_STA_PASSWORD   ""
#define DEFAULT_AP_SSID        "BOSS1-AP"

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct
{
  char ssid[CONFIG_WIFI_SSID_MAX_LEN + 1];
  char password[CONFIG_WIFI_PASSWORD_MAX_LEN + 1];
} wifi_sta_config_t;

typedef struct
{
  char ssid[CONFIG_WIFI_SSID_MAX_LEN + 1];
} wifi_ap_config_t;

typedef struct
{
  int max_users;
  int lockout_threshold;
  int lockout_duration;
  int session_timeout;
} userinfo_config_t;

typedef struct
{
  wifi_sta_config_t sta;
  wifi_ap_config_t ap;
  userinfo_config_t userinfo;
} default_config_data_t;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static FAR cJSON *g_config_json = NULL;
static uv_mutex_t g_config_mutex;
static bool g_mutex_initialized = false;

static int config_write_json_to_file(FAR cJSON *root);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: config_read_file
 *
 * Description:
 *   Read config file content.
 *
 * Input Parameters:
 *   buf   - Buffer to store content
 *   len   - Buffer size
 *
 * Returned Value:
 *   Number of bytes read, or -1 on error.
 *
 ****************************************************************************/

static ssize_t config_read_file(FAR char *buf, size_t len)
{
  int fd;
  ssize_t nread;

  fd = open(CONFIG_FILE_PATH, O_RDONLY);
  if (fd < 0)
    {
      return -1;
    }

  nread = read(fd, buf, len - 1);
  if (nread > 0)
    {
      buf[nread] = '\0';
    }

  close(fd);
  return nread;
}

/****************************************************************************
 * Name: config_write_file
 *
 * Description:
 *   Write config file content.
 *
 * Input Parameters:
 *   buf   - Content to write
 *   len   - Content length
 *
 * Returned Value:
 *   Number of bytes written, or -1 on error.
 *
 ****************************************************************************/

static ssize_t config_write_file(FAR const char *buf, size_t len)
{
  int fd;
  ssize_t nwritten;

  fd = open(CONFIG_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
    {
      return -1;
    }

  nwritten = write(fd, buf, len);
  close(fd);
  return nwritten;
}

/****************************************************************************
 * Name: config_parse_json
 *
 * Description:
 *   Parse JSON string to cJSON object.
 *
 * Input Parameters:
 *   json - JSON string
 *
 * Returned Value:
 *   Returns a pointer to the cJSON object on success;
 *   NULL on failure.
 *
 ****************************************************************************/

static FAR void *config_parse_json(FAR const char *json)
{
  FAR cJSON *root;

  root = cJSON_Parse(json);
  if (!root)
    {
      return NULL;
    }

  return root;
}

/****************************************************************************
 * Name: config_generate_default_json_wifi
 *
 * Description:
 *   Generate default WiFi JSON section.
 *
 * Input Parameters:
 *   wifi   - cJSON WiFi object
 *   config - Default config data
 *
 ****************************************************************************/

static void config_generate_default_json_wifi(FAR cJSON *wifi, FAR default_config_data_t *config)
{
  FAR cJSON *sta;
  FAR cJSON *ap;

  sta = cJSON_CreateObject();
  if (sta)
    {
      cJSON_AddStringToObject(sta, "ssid", config->sta.ssid);

      if (strlen(config->sta.password) > 0)
        {
          cJSON_AddStringToObject(sta, "password", config->sta.password);
        }
      else
        {
          cJSON_AddStringToObject(sta, "password", "");
        }

      cJSON_AddItemToObject(wifi, "sta", sta);
    }

  ap = cJSON_CreateObject();
  if (ap)
    {
      cJSON_AddStringToObject(ap, "ssid", config->ap.ssid);
      cJSON_AddItemToObject(wifi, "ap", ap);
    }
}

/****************************************************************************
 * Name: config_generate_default_json_userinfo
 *
 * Description:
 *   Generate default userinfo JSON section.
 *
 * Input Parameters:
 *   userinfo - cJSON userinfo object
 *   config   - Default config data
 *
 ****************************************************************************/

static void config_generate_default_json_userinfo(FAR cJSON *userinfo, FAR default_config_data_t *config)
{
  FAR cJSON *accounts;

  cJSON_AddNumberToObject(userinfo, "max_users", config->userinfo.max_users);
  cJSON_AddNumberToObject(userinfo, "lockout_threshold",
                          config->userinfo.lockout_threshold);
  cJSON_AddNumberToObject(userinfo, "lockout_duration",
                          config->userinfo.lockout_duration);
  cJSON_AddNumberToObject(userinfo, "session_timeout",
                          config->userinfo.session_timeout);

  accounts = cJSON_CreateArray();
  if (accounts)
    {
      cJSON_AddItemToObject(userinfo, "accounts", accounts);
    }
}

/****************************************************************************
 * Name: config_create_default_config
 *
 * Description:
 *   Create and initialize default config structure.
 *
 * Returned Value:
 *   Pointer to default config, NULL on failure.
 *
 ****************************************************************************/

static FAR default_config_data_t *config_create_default_config(void)
{
  FAR default_config_data_t *config;

  config = malloc(sizeof(default_config_data_t));
  if (!config)
    {
      return NULL;
    }

  memset(config, 0, sizeof(default_config_data_t));
  strncpy(config->sta.ssid, DEFAULT_STA_SSID, sizeof(config->sta.ssid) - 1);
  strncpy(config->sta.password, DEFAULT_STA_PASSWORD, sizeof(config->sta.password) - 1);
  strncpy(config->ap.ssid, DEFAULT_AP_SSID, sizeof(config->ap.ssid) - 1);
  config->userinfo.max_users = DEFAULT_MAX_USERS;
  config->userinfo.lockout_threshold = DEFAULT_LOCKOUT_THRESHOLD;
  config->userinfo.lockout_duration = DEFAULT_LOCKOUT_DURATION;
  config->userinfo.session_timeout = DEFAULT_SESSION_TIMEOUT;

  return config;
}

/****************************************************************************
 * Name: config_generate_default_json
 *
 * Description:
 *   Generate default JSON object from config data.
 *
 * Input Parameters:
 *   config - Default config data
 *
 * Returned Value:
 *   Pointer to cJSON object, NULL on failure.
 *
 ****************************************************************************/

static FAR void *config_generate_default_json(FAR default_config_data_t *config)
{
  FAR cJSON *root;
  FAR cJSON *wifi;
  FAR cJSON *userinfo;

  root = cJSON_CreateObject();
  if (!root)
    {
      return NULL;
    }

  cJSON_AddNumberToObject(root, "version", 1);

  wifi = cJSON_CreateObject();
  if (!wifi)
    {
      cJSON_Delete(root);
      return NULL;
    }

  cJSON_AddItemToObject(root, "wifi", wifi);

  config_generate_default_json_wifi(wifi, config);

  userinfo = cJSON_CreateObject();
  if (!userinfo)
    {
      cJSON_Delete(root);
      return NULL;
    }

  cJSON_AddItemToObject(root, "userinfo", userinfo);

  config_generate_default_json_userinfo(userinfo, config);

  return root;
}

/****************************************************************************
 * Name: config_create_default_and_save
 *
 * Description:
 *   Create default config and save to file.
 *
 * Returned Value:
 *   Pointer to cJSON object, NULL on failure.
 *
 ****************************************************************************/

static FAR cJSON *config_create_default_and_save(void)
{
  FAR cJSON *root;
  FAR default_config_data_t *config;
  int ret;

  CFG_LOG("Creating default config and saving to file");

  config = config_create_default_config();
  if (!config)
    {
      CFG_ERR("Failed to create default config data");
      return NULL;
    }

  root = config_generate_default_json(config);
  if (!root)
    {
      CFG_ERR("Failed to generate default JSON");
      free(config);
      return NULL;
    }
  free(config);

  g_config_json = root;

  CFG_LOG("Default config created, now saving to file");

  ret = config_write_json_to_file(root);
  if (ret < 0)
    {
      CFG_ERR("Failed to save default config: %d", ret);
      cJSON_Delete(root);
      g_config_json = NULL;
      return NULL;
    }

  CFG_LOG("Default config saved successfully");
  return root;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: config_load
 *
 * Description:
 *   Load configuration from file.
 *
 ****************************************************************************/

FAR void *config_load(void)
{
  ssize_t nread;
  struct stat st;
  size_t buf_size = CONFIG_JSON_DEFAULT_SIZE;
  FAR void *ret;
  FAR char *json_str = NULL;
  FAR char *encrypted_data = NULL;

  if (!g_mutex_initialized)
    {
      uv_mutex_init(&g_config_mutex);
      g_mutex_initialized = true;
    }

  uv_mutex_lock(&g_config_mutex);

  if (g_config_json)
    {
      uv_mutex_unlock(&g_config_mutex);
      return g_config_json;
    }

  if (stat(CONFIG_FILE_PATH, &st) == 0 && st.st_size > 1)
    {
      buf_size = st.st_size + 1;
    }

  json_str = malloc(buf_size);
  if (!json_str)
    {
      ret = NULL;
      goto cleanup_mutex;
    }

#ifndef CONFIG_CONFIG_FILE_DEBUG
  encrypted_data = malloc(buf_size);
  if (!encrypted_data)
    {
      ret = NULL;
      goto cleanup_json_str;
    }
#endif

#ifdef CONFIG_CONFIG_FILE_DEBUG
  nread = config_read_file(json_str, buf_size);
#else
  nread = config_read_file(encrypted_data, buf_size);
#endif
  if (nread < 0)
    {
      g_config_json = config_create_default_and_save();
      if (g_config_json)
        {
          ret = g_config_json;
        }
      else
        {
          ret = NULL;
        }

      goto cleanup_encrypted_data;
    }

#ifndef CONFIG_CONFIG_FILE_DEBUG
  {
    int decrypt_ret = config_decrypt(encrypted_data, json_str, buf_size);
    if (decrypt_ret < 0)
      {
        ret = NULL;
        goto cleanup_encrypted_data;
      }
  }
#endif

  g_config_json = (FAR cJSON *)config_parse_json(json_str);
  ret = g_config_json;

cleanup_encrypted_data:
  free(encrypted_data);

cleanup_json_str:
  free(json_str);

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);
  return ret;
}

/****************************************************************************
 * Name: config_save
 *
 * Description:
 *   Save configuration to file.
 *
 ****************************************************************************/

static int config_write_json_to_file(FAR cJSON *root)
{
  char *json_str = NULL;
  ssize_t nwritten;
  int ret = OK;

  json_str = cJSON_PrintUnformatted(root);
  if (!json_str)
    {
      CFG_ERR("Failed to print JSON");
      return -ENOMEM;
    }

#ifdef CONFIG_CONFIG_FILE_DEBUG
  nwritten = config_write_file(json_str, strlen(json_str));
  if (nwritten < 0)
    {
      CFG_ERR("Failed to write config file: %zd", nwritten);
      ret = (int)nwritten;
      goto cleanup;
    }
#else
  size_t encrypted_data_len = config_encrypted_len(strlen(json_str));
  char *encrypted_data = malloc(encrypted_data_len);
  if (!encrypted_data)
    {
      CFG_ERR("Failed to allocate encrypted data buffer");
      ret = -ENOMEM;
      goto cleanup;
    }

  ret = config_encrypt(json_str, encrypted_data, encrypted_data_len);
  if (ret < 0)
    {
      CFG_ERR("Failed to encrypt config");
      free(encrypted_data);
      goto cleanup;
    }

  nwritten = config_write_file(encrypted_data, strlen(encrypted_data));
  if (nwritten < 0)
    {
      CFG_ERR("Failed to write encrypted config: %zd", nwritten);
      ret = (int)nwritten;
      free(encrypted_data);
      goto cleanup;
    }

  free(encrypted_data);
#endif

  ret = OK;

cleanup:
  free(json_str);
  return ret;
}

int config_save(FAR void *json_obj)
{
  FAR cJSON *root = NULL;
  int ret = OK;

  uv_mutex_lock(&g_config_mutex);

  root = (FAR cJSON *)json_obj;
  if (!root)
    {
      CFG_ERR("invalid json_obj");
      ret = -EINVAL;
      goto cleanup_mutex;
    }

  ret = config_write_json_to_file(root);
  if (ret < 0)
    {
      CFG_ERR("Failed to write config file: %d", ret);
    }

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);
  return ret;
}

void config_free(FAR void *json_obj)
{
  FAR cJSON *root = NULL;

  uv_mutex_lock(&g_config_mutex);
  root = (FAR cJSON *)json_obj;
  if (root)
    {
      cJSON_Delete(root);
    }

  g_config_json = NULL;
  uv_mutex_unlock(&g_config_mutex);

  if (g_mutex_initialized)
    {
      uv_mutex_destroy(&g_config_mutex);
      g_mutex_initialized = false;
    }
}

/****************************************************************************
 * Name: config_get_wifi_sta
 *
 * Description:
 *   Get WiFi STA configuration.
 *
 ****************************************************************************/

int config_get_wifi_sta(FAR char *ssid, FAR char *password, size_t maxlen)
{
  FAR cJSON *root = NULL;
  FAR cJSON *wifi;
  FAR cJSON *sta;
  FAR cJSON *ssid_item;
  FAR cJSON *password_item;
  int ret = 0;

  if (!ssid || !password || maxlen == 0)
    {
      CFG_ERR("invalid parameters");
      return -EINVAL;
    }

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  wifi = cJSON_GetObjectItem(root, "wifi");
  if (!wifi)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  sta = cJSON_GetObjectItem(wifi, "sta");
  if (!sta)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  ssid_item = cJSON_GetObjectItem(sta, "ssid");
  if (ssid_item && ssid_item->valuestring)
    {
      strncpy(ssid, ssid_item->valuestring, maxlen - 1);
      ssid[maxlen - 1] = '\0';
    }
  else
    {
      ssid[0] = '\0';
    }

  password_item = cJSON_GetObjectItem(sta, "password");
  if (password_item && password_item->valuestring)
    {
      strncpy(password, password_item->valuestring, maxlen - 1);
      password[maxlen - 1] = '\0';
    }
  else
    {
      password[0] = '\0';
    }
  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_set_wifi_sta
 *
 * Description:
 *   Set WiFi STA configuration.
 *
 ****************************************************************************/

int config_set_wifi_sta(FAR const char *ssid, FAR const char *password)
{
  FAR cJSON *root = NULL;
  FAR cJSON *wifi;
  FAR cJSON *sta;
  FAR cJSON *ssid_item;
  FAR cJSON *password_item;
  int ret = 0;

  if (!ssid)
    {
      CFG_ERR("ssid is null");
      return -EINVAL;
    }

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  wifi = cJSON_GetObjectItem(root, "wifi");
  if (!wifi)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  sta = cJSON_GetObjectItem(wifi, "sta");
  if (!sta)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  ssid_item = cJSON_GetObjectItem(sta, "ssid");
  if (ssid_item)
    {
      cJSON_DeleteItemFromObject(sta, "ssid");
    }
  cJSON_AddStringToObject(sta, "ssid", ssid);

  if (password)
    {
      password_item = cJSON_GetObjectItem(sta, "password");
      if (password_item)
        {
          cJSON_DeleteItemFromObject(sta, "password");
        }

      cJSON_AddStringToObject(sta, "password", password);
    }

  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_get_wifi_ap_ssid
 *
 * Description:
 *   Get WiFi AP SSID.
 *
 ****************************************************************************/

int config_get_wifi_ap_ssid(FAR char *ssid, size_t maxlen)
{
  FAR cJSON *root = NULL;
  FAR cJSON *wifi;
  FAR cJSON *ap;
  FAR cJSON *ssid_item;
  int ret = 0;

  if (!ssid || maxlen == 0)
    {
      CFG_ERR("invalid parameters");
      return -EINVAL;
    }

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  wifi = cJSON_GetObjectItem(root, "wifi");
  if (!wifi)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  ap = cJSON_GetObjectItem(wifi, "ap");
  if (!ap)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  ssid_item = cJSON_GetObjectItem(ap, "ssid");
  if (ssid_item && ssid_item->valuestring)
    {
      strncpy(ssid, ssid_item->valuestring, maxlen - 1);
      ssid[maxlen - 1] = '\0';
    }
  else
    {
      ssid[0] = '\0';
    }

  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_set_wifi_ap_ssid
 *
 * Description:
 *   Set WiFi AP SSID.
 *
 ****************************************************************************/

int config_set_wifi_ap_ssid(FAR const char *ssid)
{
  FAR cJSON *root = NULL;
  FAR cJSON *wifi;
  FAR cJSON *ap;
  FAR cJSON *ssid_item;
  int ret = 0;

  if (!ssid)
    {
      CFG_ERR("ssid is null");
      return -EINVAL;
    }

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  wifi = cJSON_GetObjectItem(root, "wifi");
  if (!wifi)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  ap = cJSON_GetObjectItem(wifi, "ap");
  if (!ap)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  ssid_item = cJSON_GetObjectItem(ap, "ssid");
  if (ssid_item)
    {
      cJSON_DeleteItemFromObject(ap, "ssid");
    }
  cJSON_AddStringToObject(ap, "ssid", ssid);

  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_get_max_users
 *
 * Description:
 *   Get max users limit.
 *
 ****************************************************************************/

int config_get_max_users(void)
{
  FAR cJSON *root = NULL;
  FAR cJSON *max_users_item;
  int ret = 0;
  int max_users = 0;

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  max_users_item = cJSON_GetObjectItem(root, "max_users");
  if (max_users_item)
    {
      max_users = max_users_item->valueint;
    }
  else
    {
      max_users = 5;
    }

  ret = max_users;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_get_userinfo
 *
 * Description:
 *   Get user security configuration (lockout_threshold,
 *   lockout_duration, session_timeout).
 *
 ****************************************************************************/

int config_get_userinfo(FAR int *threshold, FAR int *duration,
                      FAR int *timeout)
{
  FAR cJSON *root = NULL;
  FAR cJSON *userinfo;
  FAR cJSON *item;
  int ret = 0;

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  userinfo = cJSON_GetObjectItem(root, "userinfo");
  if (!userinfo)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  if (threshold)
    {
      item = cJSON_GetObjectItem(userinfo, "lockout_threshold");
      if (item)
        {
          *threshold = item->valueint;
        }
    }

  if (duration)
    {
      item = cJSON_GetObjectItem(userinfo, "lockout_duration");
      if (item)
        {
          *duration = item->valueint;
        }
    }

  if (timeout)
    {
      item = cJSON_GetObjectItem(userinfo, "session_timeout");
      if (item)
        {
          *timeout = item->valueint;
        }
    }

  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_set_userinfo
 *
 * Description:
 *   Set user security configuration (lockout_threshold,
 *   lockout_duration, session_timeout).
 *   Use -1 to keep current value.
 *
 ****************************************************************************/

int config_set_userinfo(int threshold, int duration, int timeout)
{
  FAR cJSON *root = NULL;
  FAR cJSON *userinfo;
  FAR cJSON *item;
  int ret = 0;

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  userinfo = cJSON_GetObjectItem(root, "userinfo");
  if (!userinfo)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  if (threshold >= 0)
    {
      item = cJSON_GetObjectItem(userinfo, "lockout_threshold");
      if (item)
        {
          item->valueint = threshold;
        }
    }

  if (duration >= 0)
    {
      item = cJSON_GetObjectItem(userinfo, "lockout_duration");
      if (item)
        {
          item->valueint = duration;
        }
    }

  if (timeout >= 0)
    {
      item = cJSON_GetObjectItem(userinfo, "session_timeout");
      if (item)
        {
          item->valueint = timeout;
        }
    }

  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_update_account_state
 *
 * Description:
 *   Update account state (online/offline). If account doesn't exist, create it.
 *
 ****************************************************************************/

int config_update_account_state(FAR const char *username, account_status_t status)
{
  FAR cJSON *root = NULL;
  FAR cJSON *userinfo;
  FAR cJSON *accounts;
  FAR cJSON *account;
  FAR cJSON *name_item;
  FAR cJSON *state_item;
  bool account_found = false;
  int ret = 0;

  if (!username)
    {
      return -EINVAL;
    }

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = -ENOENT;
      goto cleanup_mutex;
    }

  userinfo = cJSON_GetObjectItem(root, "userinfo");
  if (!userinfo)
    {
      ret = -ENOMEM;
      goto cleanup_mutex;
    }

  accounts = cJSON_GetObjectItem(userinfo, "accounts");
  if (!accounts)
    {
      accounts = cJSON_CreateArray();
      if (!accounts)
        {
          ret = -ENOMEM;
          goto cleanup_mutex;
        }
      cJSON_AddItemToObject(userinfo, "accounts", accounts);
    }

  cJSON_ArrayForEach(account, accounts)
    {
      name_item = cJSON_GetObjectItem(account, "username");
      if (name_item && name_item->valuestring &&
          strncmp(name_item->valuestring, username, CONFIG_USERNAME_MAX_LEN) == 0)
        {
          account_found = true;
          break;
        }
    }

  if (!account_found)
    {
      account = cJSON_CreateObject();
      if (!account)
        {
          ret = -ENOMEM;
          goto cleanup_mutex;
      }

      cJSON_AddStringToObject(account, "username", username);
      cJSON_AddNumberToObject(account, "login_status", status);
      cJSON_AddItemToArray(accounts, account);
    }
  else
    {
      state_item = cJSON_GetObjectItem(account, "login_status");
      if (state_item)
        {
          state_item->valueint = status;
        }
    }

  ret = OK;

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}

/****************************************************************************
 * Name: config_get_account_state
 *
 * Description:
 *   Get account state.
 *
 ****************************************************************************/

account_status_t config_get_account_state(FAR const char *username)
{
  FAR cJSON *root = NULL;
  FAR cJSON *userinfo;
  FAR cJSON *accounts;
  FAR cJSON *account;
  int ret = 0;

  if (!username)
    {
      return ACCOUNT_STATUS_INVALID;
    }

  uv_mutex_lock(&g_config_mutex);
  root = g_config_json;
  if (!root)
    {
      ret = ACCOUNT_STATUS_INVALID;
      goto cleanup_mutex;
    }

  userinfo = cJSON_GetObjectItem(root, "userinfo");
  if (!userinfo)
    {
      ret = ACCOUNT_STATUS_INVALID;
      goto cleanup_mutex;
    }

  accounts = cJSON_GetObjectItem(userinfo, "accounts");
  if (!accounts)
    {
      ret = ACCOUNT_STATUS_INVALID;
      goto cleanup_mutex;
    }

  ret = ACCOUNT_STATUS_INVALID;
  cJSON_ArrayForEach(account, accounts)
    {
      cJSON *name_item = cJSON_GetObjectItem(account, "username");
      if (name_item && name_item->valuestring &&
          strncmp(name_item->valuestring, username, CONFIG_USERNAME_MAX_LEN) == 0)
        {
          cJSON *state_item = cJSON_GetObjectItem(account, "login_status");
          if (state_item)
            {
              ret = state_item->valueint;
            }
        }
    }

cleanup_mutex:
  uv_mutex_unlock(&g_config_mutex);

  return ret;
}
