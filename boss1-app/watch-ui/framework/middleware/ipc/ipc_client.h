/****************************************************************************
 * apps/watch-ui/ipc/ipc_client.h
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

#ifndef __APPS_WATCH_UI_IPC_IPC_CLIENT_H
#define __APPS_WATCH_UI_IPC_IPC_CLIENT_H

#include <nuttx/config.h>
#include <stddef.h>

#include "ipc/ipc_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CONFIG */
int ipc_config_get_version(uint32_t *version);
int ipc_config_get_sta(char *ssid, char *passwd, size_t len);
int ipc_config_get_ap(char *ssid, size_t len);
int ipc_config_get_max_users(uint32_t *max_users);
int ipc_config_get_userinfo(int *lockout_threshold, int *lockout_duration,
                            int *session_timeout);
int ipc_config_get_account(char *username, int *status);

/* CONFIG SET */
int ipc_config_set_sta(const char *ssid, const char *passwd);
int ipc_config_set_ap(const char *ssid);
int ipc_config_set_userinfo(int lockout_threshold, int lockout_duration,
                            int session_timeout);
int ipc_config_set_account(const char *username, int status);



/* APWIFI */
// int ipc_apwifi_get_ssid(FAR char *ssid, size_t maxlen);
// int ipc_apwifi_set_ssid(FAR const char *ssid);

/* STAWIFI */
// int ipc_sta_get_info(FAR char *ssid, FAR char *password, size_t maxlen);
// int ipc_sta_set_info(FAR const char *ssid, FAR const char *password);
// int ipc_sta_connect(void);
// int ipc_sta_disconnect(void);
// int ipc_sta_get_status(FAR int *status);

/* Low-level request/response */
ipc_response_t *ipc_send_request(FAR ipc_request_t *req);


#ifdef __cplusplus
}
#endif

#endif
