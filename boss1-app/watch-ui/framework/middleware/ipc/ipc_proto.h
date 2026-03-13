/****************************************************************************
 * apps/watch-ui/ipc/ipc_proto.h
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

#ifndef __APPS_WATCH_UI_IPC_IPC_PROTO_H
#define __APPS_WATCH_UI_IPC_IPC_PROTO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef CONFIG_IPC_SOCKET_PATH
#define CONFIG_IPC_SOCKET_PATH     "/var/run/ipc.sock"
#endif

#ifndef CONFIG_IPC_MAX_REQUEST_LEN
#define CONFIG_IPC_MAX_REQUEST_LEN 4096
#endif

#ifndef CONFIG_IPC_RESPONSE_MAX_DATA_LEN
#define CONFIG_IPC_RESPONSE_MAX_DATA_LEN 4096
#endif

#pragma pack(push, 1)

/****************************************************************************
 * IPC Magic Number
 ****************************************************************************/

#define IPC_MAGIC  0xAABBCCDD

/****************************************************************************
 * IPC Error Code
 ****************************************************************************/

enum ipc_error
{
  IPC_OK = 0,
  IPC_ERROR_INVALID_PARAM = -1,
  IPC_ERROR_INVALID_MSG = -2,
  IPC_ERROR_NO_MEMORY = -3,
  IPC_ERROR_NOT_FOUND = -4,
};

/****************************************************************************
 * IPC Message Type (high 16 bits)
 ****************************************************************************/

enum ipc_msg_type
{
  IPC_TYPE_CONFIG = 0x0001,
  IPC_TYPE_APWIFI = 0x0002,
  IPC_TYPE_STAWIFI = 0x0003,
};

/****************************************************************************
 * IPC Message ID (32-bit: high 16 bits = type, low 16 bits = id)
 ****************************************************************************/

enum ipc_msg_id
{
  /* CONFIG - 配置消息 */
  IPC_CONFIG_TYPE = (IPC_TYPE_CONFIG << 16),

  /* CONFIG GET 细分 */
  IPC_CONFIG_GET,                  /* 获取完整配置 */
  IPC_CONFIG_GET_VERSION,         /* 获取配置版本号 */
  IPC_CONFIG_GET_WIFI_STA,         /* 获取 WiFi STA 配置 */
  IPC_CONFIG_GET_WIFI_AP,         /* 获取 WiFi AP SSID */
  IPC_CONFIG_GET_MAX_USERS,       /* 获取最大用户数 */
  IPC_CONFIG_GET_USERINFO,        /* 获取用户安全配置 */
  IPC_CONFIG_GET_ACCOUNT,         /* 获取指定账户状态 */

  /* CONFIG SET 细分 */
  IPC_CONFIG_SET,                  /* 设置配置 */
  IPC_CONFIG_SET_WIFI_STA,        /* 设置 WiFi STA 配置 */
  IPC_CONFIG_SET_WIFI_AP,         /* 设置 WiFi AP SSID */
  IPC_CONFIG_SET_USERINFO,         /* 设置用户安全配置 */
  IPC_CONFIG_SET_ACCOUNT,          /* 设置账户状态 */
  IPC_CONFIG_RELOAD,               /* 重新加载配置 */

  /* APWIFI - AP WiFi 动作消息 */
  IPC_APWIFI_TYPE = (IPC_TYPE_APWIFI << 16),
  IPC_APWIFI_GET_SSID,
  IPC_APWIFI_SET_SSID,

  /* STAWIFI - STA WiFi 动作消息 */
  IPC_STAWIFI_TYPE = (IPC_TYPE_STAWIFI << 16),
  IPC_STA_GET_INFO,
  IPC_STA_SET_INFO,
  IPC_STA_CONNECT,
  IPC_STA_DISCONNECT,
  IPC_STA_GET_STATUS,
};

#define IPC_GET_MSG_TYPE(msg_id)  ((msg_id) >> 16)

/****************************************************************************
 * IPC Data Structures
 ****************************************************************************/

#define IPC_WIFI_SSID_MAX     32
#define IPC_WIFI_PASSWORD_MAX 64
#define IPC_USERNAME_MAX       32

typedef struct
{
  uint32_t version;
} ipc_config_version_t;

typedef struct
{
  char ssid[IPC_WIFI_SSID_MAX];
  char password[IPC_WIFI_PASSWORD_MAX];
} ipc_config_sta_info_t;

typedef struct
{
  char ssid[IPC_WIFI_SSID_MAX];
} ipc_config_ap_info_t;

typedef struct
{
  uint32_t max_users;
} ipc_config_max_users_t;

typedef struct
{
  int lockout_threshold;
  int lockout_duration;
  int session_timeout;
} ipc_config_userinfo_t;

typedef struct
{
  char username[IPC_USERNAME_MAX];
} ipc_account_req_t;

typedef struct
{
  char username[IPC_USERNAME_MAX];
  int login_status;
} ipc_config_account_info_t;

/****************************************************************************
 * IPC Structures
 ****************************************************************************/

typedef struct
{
  uint32_t magic;
  uint32_t msg_id;
  uint32_t data_len;
  uint32_t checksum;
  char data[];
} ipc_request_t;

typedef struct
{
  uint32_t magic;
  int32_t result;
  uint32_t data_len;
  uint32_t checksum;
  char data[];
} ipc_response_t;

#pragma pack(pop)

/****************************************************************************
 * IPC Checksum Functions
 ****************************************************************************/

uint32_t ipc_calculate_checksum(const void *data, size_t len);
bool ipc_verify_request_checksum(const ipc_request_t *req);
bool ipc_verify_response_checksum(const ipc_response_t *resp);

#ifdef __cplusplus
}
#endif

#endif
