/****************************************************************************
 * apps/watch-ui/framework/middleware/ipc/ipc_client/ipc_client_config.c
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

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdlib.h>
#include <string.h>

#include "ipc/ipc_proto.h"
#include "ipc/ipc_client.h"
#include <watchui/log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_IPC_DEBUG
#  define IPC_LOG(fmt, ...)    appinfo("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_DBG(fmt, ...)    appdbg("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_WARN(fmt, ...)   appwarn("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_ERR(fmt, ...)    apperr("[IPC] " fmt, ##__VA_ARGS__)
#else
#  define IPC_LOG(fmt, ...)
#  define IPC_DBG(fmt, ...)
#  define IPC_WARN(fmt, ...)   appwarn("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_ERR(fmt, ...)    apperr("[IPC] " fmt, ##__VA_ARGS__)
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static ipc_response_t *ipc_config_request(uint32_t msg_id, FAR const void *data,
                                           size_t data_len)
{
  size_t req_len = sizeof(ipc_request_t) + data_len;
  ipc_request_t *req = (ipc_request_t *)malloc(req_len);
  if (!req)
    {
      IPC_ERR("failed to allocate request");
      return NULL;
    }

  req->magic = IPC_MAGIC;
  req->msg_id = msg_id;
  req->data_len = data_len;
  if (data && data_len > 0)
    {
      memcpy(req->data, data, data_len);
    }

  ipc_response_t *resp = ipc_send_request(req);
  free(req);

  if (!resp)
    {
      IPC_ERR("ipc_send_request returned NULL");
      return NULL;
    }

  return resp;
}

/****************************************************************************
 * Public Functions - CONFIG
 ****************************************************************************/

int ipc_config_get_version(uint32_t *version)
{
  ipc_config_version_t data;
  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_GET_VERSION, NULL, 0);
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  if (result == IPC_OK) {
    memcpy(&data, resp->data, sizeof(data));
    *version = data.version;
  }

  free(resp);
  return result;
}

int ipc_config_get_sta(char *ssid, char *passwd, size_t len)
{
  ipc_config_sta_info_t data;
  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_GET_WIFI_STA, NULL, 0);
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  if (result == IPC_OK)
    {
      memcpy(&data, resp->data, sizeof(data));
      strncpy(ssid, data.ssid, len);
      ssid[len - 1] = '\0';
      strncpy(passwd, data.password, len);
      passwd[len - 1] = '\0';
    }

  free(resp);
  return result;
}

int ipc_config_get_ap(char *ssid, size_t len)
{
  ipc_config_ap_info_t data;
  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_GET_WIFI_AP, NULL, 0);
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  if (result == IPC_OK)
    {
      memcpy(&data, resp->data, sizeof(data));
      strncpy(ssid, data.ssid, len);
      ssid[len - 1] = '\0';
    }

  free(resp);
  return result;
}

int ipc_config_get_max_users(uint32_t *max_users)
{
  ipc_config_max_users_t data;
  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_GET_MAX_USERS, NULL, 0);
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  if (result == IPC_OK)
    {
      memcpy(&data, resp->data, sizeof(data));
      *max_users = data.max_users;
    }

  free(resp);
  return result;
}

int ipc_config_get_userinfo(int *lockout_threshold, int *lockout_duration,
                            int *session_timeout)
{
  ipc_config_userinfo_t data;
  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_GET_USERINFO, NULL, 0);
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  if (result == IPC_OK)
    {
      memcpy(&data, resp->data, sizeof(data));
      *lockout_threshold = data.lockout_threshold;
      *lockout_duration = data.lockout_duration;
      *session_timeout = data.session_timeout;
    }

  free(resp);
  return result;
}

int ipc_config_get_account(char *username, int *status)
{
  ipc_account_req_t req_data;
  ipc_config_account_info_t resp_data;

  memcpy(req_data.username, username, IPC_USERNAME_MAX);
  req_data.username[IPC_USERNAME_MAX - 1] = '\0';

  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_GET_ACCOUNT, &req_data, sizeof(req_data));
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  if (result == IPC_OK)
    {
      memcpy(&resp_data, resp->data, sizeof(resp_data));
      *status = resp_data.login_status;
    }

  free(resp);
  return result;
}

/****************************************************************************
 * Public Functions - CONFIG SET
 ****************************************************************************/

int ipc_config_set_sta(const char *ssid, const char *passwd)
{
  ipc_config_sta_info_t data;
  memset(&data, 0, sizeof(data));
  strncpy(data.ssid, ssid, IPC_WIFI_SSID_MAX - 1);
  strncpy(data.password, passwd, IPC_WIFI_PASSWORD_MAX - 1);

  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_SET_WIFI_STA, &data, sizeof(data));
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  free(resp);
  return result;
}

int ipc_config_set_ap(const char *ssid)
{
  ipc_config_ap_info_t data;
  memset(&data, 0, sizeof(data));
  strncpy(data.ssid, ssid, IPC_WIFI_SSID_MAX - 1);

  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_SET_WIFI_AP, &data, sizeof(data));
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  free(resp);
  return result;
}

int ipc_config_set_userinfo(int lockout_threshold, int lockout_duration,
                            int session_timeout)
{
  ipc_config_userinfo_t data;
  data.lockout_threshold = lockout_threshold;
  data.lockout_duration = lockout_duration;
  data.session_timeout = session_timeout;

  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_SET_USERINFO, &data, sizeof(data));
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  free(resp);
  return result;
}

int ipc_config_set_account(const char *username, int status)
{
  ipc_config_account_info_t data;
  memset(&data, 0, sizeof(data));
  strncpy(data.username, username, IPC_USERNAME_MAX - 1);
  data.login_status = status;

  ipc_response_t *resp = ipc_config_request(IPC_CONFIG_SET_ACCOUNT, &data, sizeof(data));
  if (!resp)
    {
      return -1;
    }

  int result = resp->result;
  free(resp);
  return result;
}
