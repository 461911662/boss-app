/****************************************************************************
 * apps/watch-ui/framework/middleware/ipc/ipc_client/ipc_client_apwifi.c
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

extern ipc_response_t *ipc_send_request(FAR const ipc_request_t *req);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static ipc_response_t *ipc_apwifi_request(uint32_t msg_id, FAR const void *data,
                                         size_t data_len)
{
  size_t req_len = sizeof(ipc_request_t) + data_len;
  ipc_request_t *req = (ipc_request_t *)malloc(req_len);
  if (!req)
    {
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
  return resp;
}

/****************************************************************************
 * Public Functions - APWIFI
 ****************************************************************************/

int ipc_apwifi_get_ssid(FAR char *ssid, size_t maxlen)
{
  ipc_response_t *resp = ipc_apwifi_request(IPC_APWIFI_GET_SSID, NULL, 0);
  if (!resp)
    {
      return -1;
    }

  if (resp->result == 0 && ssid && maxlen > 0 && resp->data_len > 0)
    {
      size_t copy_len = resp->data_len < maxlen ? resp->data_len : maxlen - 1;
      memcpy(ssid, resp->data, copy_len);
      ssid[copy_len] = '\0';
    }

  int ret = resp->result;
  free(resp);
  return ret;
}

int ipc_apwifi_set_ssid(FAR const char *ssid)
{
  ipc_ap_info_t data;
  memset(&data, 0, sizeof(data));
  if (ssid)
    {
      strncpy(data.ssid, ssid, sizeof(data.ssid) - 1);
    }

  ipc_response_t *resp = ipc_apwifi_request(IPC_APWIFI_SET_SSID, &data, sizeof(data));
  if (!resp)
    {
      return -1;
    }

  int ret = resp->result;
  free(resp);
  return ret;
}
