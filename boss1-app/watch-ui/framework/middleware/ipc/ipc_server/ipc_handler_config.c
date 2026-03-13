/****************************************************************************
 * apps/watch-ui/framework/middleware/ipc/ipc_server/ipc_handler_config.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <uv.h>

#include "pipeline/pipeline.h"
#include "ipc/ipc_proto.h"
#include "ipc/ipc_server.h"
#include "config/config_manager.h"
#include <watchui/log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_IPC_DEBUG
#  define IPC_LOG(fmt, ...)    appinfo("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_DBG(fmt, ...)    appdbg("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_WARN(fmt, ...)  appwarn("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_ERR(fmt, ...)   apperr("[IPC] " fmt, ##__VA_ARGS__)
#else
#  define IPC_LOG(fmt, ...)
#  define IPC_DBG(fmt, ...)
#  define IPC_WARN(fmt, ...)  appwarn("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_ERR(fmt, ...)   apperr("[IPC] " fmt, ##__VA_ARGS__)
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct
{
  ipc_client_t *client;
  ipc_request_t *req;
  void *resp_data;
  size_t resp_data_len;
  int32_t result;
} ipc_config_ctx_t;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void ipc_config_ctx_free(ipc_config_ctx_t *ctx)
{
  if (ctx)
    {
      if (ctx->req)
        {
          free(ctx->req);
        }
      if (ctx->resp_data)
        {
          free(ctx->resp_data);
        }
      free(ctx);
    }
}

static void ipc_config_pipeline_complete(pipeline_t *p, bool success,
                                         void *user_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)user_data;
  ipc_response_t *resp = NULL;

  if (success && ctx->result == IPC_OK)
    {
      resp = ipc_create_success_resp(ctx->resp_data, ctx->resp_data_len);
    }
  else
    {
      resp = ipc_create_error_resp(ctx->result);
    }

  if (ctx->client)
    {
      ipc_send_response(ctx->client, resp);
    }
  else
    {
      IPC_ERR("ctx->client is NULL!");
    }

  pipeline_destroy_async(p);
  ipc_config_ctx_free(ctx);
}

static node_result_t ipc_config_node_get_version(pipeline_node_t *node,
                                                  int event_id,
                                                  void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  uint32_t version = 1;
  ctx->resp_data = malloc(sizeof(ipc_config_version_t));
  if (!ctx->resp_data)
    {
      ctx->result = IPC_ERROR_NO_MEMORY;
      return NODE_FAILED;
    }

  memcpy(ctx->resp_data, &version, sizeof(ipc_config_version_t));
  ctx->resp_data_len = sizeof(ipc_config_version_t);
  ctx->result = IPC_OK;

  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_get_wifi_sta(pipeline_node_t *node,
                                                    int event_id,
                                                    void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  ctx->resp_data = malloc(sizeof(ipc_config_sta_info_t));
  if (!ctx->resp_data)
    {
      ctx->result = IPC_ERROR_NO_MEMORY;
      return NODE_FAILED;
    }

  ipc_config_sta_info_t *sta = (ipc_config_sta_info_t *)ctx->resp_data;
  int ret = config_get_wifi_sta(sta->ssid, sta->password, IPC_WIFI_SSID_MAX);
  if (ret < 0)
    {
      free(ctx->resp_data);
      ctx->resp_data = NULL;
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->resp_data_len = sizeof(ipc_config_sta_info_t);
  ctx->result = IPC_OK;

  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_get_wifi_ap(pipeline_node_t *node,
                                                   int event_id,
                                                   void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  ctx->resp_data = malloc(sizeof(ipc_config_ap_info_t));
  if (!ctx->resp_data)
    {
      ctx->result = IPC_ERROR_NO_MEMORY;
      return NODE_FAILED;
    }

  ipc_config_ap_info_t *ap = (ipc_config_ap_info_t *)ctx->resp_data;
  int ret = config_get_wifi_ap_ssid(ap->ssid, IPC_WIFI_SSID_MAX);
  if (ret < 0)
    {
      free(ctx->resp_data);
      ctx->resp_data = NULL;
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->resp_data_len = sizeof(ipc_config_ap_info_t);
  ctx->result = IPC_OK;

  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_get_max_users(pipeline_node_t *node,
                                                    int event_id,
                                                    void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  ctx->resp_data = malloc(sizeof(ipc_config_max_users_t));
  if (!ctx->resp_data)
    {
      ctx->result = IPC_ERROR_NO_MEMORY;
      return NODE_FAILED;
    }

  ipc_config_max_users_t *max_users = (ipc_config_max_users_t *)ctx->resp_data;
  int ret = config_get_max_users();
  if (ret < 0)
    {
      free(ctx->resp_data);
      ctx->resp_data = NULL;
      ctx->result = ret;
      return NODE_FAILED;
    }

  max_users->max_users = (uint32_t)ret;
  ctx->resp_data_len = sizeof(ipc_config_max_users_t);
  ctx->result = IPC_OK;

  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_get_userinfo(pipeline_node_t *node,
                                                    int event_id,
                                                    void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  ctx->resp_data = malloc(sizeof(ipc_config_userinfo_t));
  if (!ctx->resp_data)
    {
      ctx->result = IPC_ERROR_NO_MEMORY;
      return NODE_FAILED;
    }

  ipc_config_userinfo_t *userinfo = (ipc_config_userinfo_t *)ctx->resp_data;
  int ret = config_get_userinfo(&userinfo->lockout_threshold,
                                &userinfo->lockout_duration,
                                &userinfo->session_timeout);
  if (ret < 0)
    {
      free(ctx->resp_data);
      ctx->resp_data = NULL;
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->resp_data_len = sizeof(ipc_config_userinfo_t);
  ctx->result = IPC_OK;

  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_get_account(pipeline_node_t *node,
                                                   int event_id,
                                                   void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  if (!ctx->req || ctx->req->data_len < sizeof(ipc_account_req_t))
    {
      ctx->result = IPC_ERROR_INVALID_PARAM;
      return NODE_FAILED;
    }

  ipc_account_req_t *req = (ipc_account_req_t *)ctx->req->data;

  ctx->resp_data = malloc(sizeof(ipc_config_account_info_t));
  if (!ctx->resp_data)
    {
      ctx->result = IPC_ERROR_NO_MEMORY;
      return NODE_FAILED;
    }

  ipc_config_account_info_t *account = (ipc_config_account_info_t *)ctx->resp_data;
  memset(account, 0, sizeof(ipc_config_account_info_t));
  strncpy(account->username, req->username, IPC_USERNAME_MAX - 1);
  account->username[IPC_USERNAME_MAX - 1] = '\0';

  account_status_t status = config_get_account_state(req->username);
  if (status == ACCOUNT_STATUS_INVALID)
    {
      free(ctx->resp_data);
      ctx->resp_data = NULL;
      ctx->result = IPC_ERROR_NOT_FOUND;
      return NODE_FAILED;
    }

  account->login_status = (int)status;
  ctx->resp_data_len = sizeof(ipc_config_account_info_t);
  ctx->result = IPC_OK;

  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_set_wifi_sta(pipeline_node_t *node,
                                                     int event_id,
                                                     void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  if (!ctx->req || ctx->req->data_len < sizeof(ipc_config_sta_info_t))
    {
      ctx->result = IPC_ERROR_INVALID_PARAM;
      return NODE_FAILED;
    }

  ipc_config_sta_info_t *sta = (ipc_config_sta_info_t *)ctx->req->data;
  int ret = config_set_wifi_sta(sta->ssid, sta->password);
  if (ret < 0)
    {
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->result = IPC_OK;
  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_set_wifi_ap(pipeline_node_t *node,
                                                    int event_id,
                                                    void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  if (!ctx->req || ctx->req->data_len < sizeof(ipc_config_ap_info_t))
    {
      ctx->result = IPC_ERROR_INVALID_PARAM;
      return NODE_FAILED;
    }

  ipc_config_ap_info_t *ap = (ipc_config_ap_info_t *)ctx->req->data;
  int ret = config_set_wifi_ap_ssid(ap->ssid);
  if (ret < 0)
    {
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->result = IPC_OK;
  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_set_userinfo(pipeline_node_t *node,
                                                     int event_id,
                                                     void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  if (!ctx->req || ctx->req->data_len < sizeof(ipc_config_userinfo_t))
    {
      ctx->result = IPC_ERROR_INVALID_PARAM;
      return NODE_FAILED;
    }

  ipc_config_userinfo_t *userinfo = (ipc_config_userinfo_t *)ctx->req->data;
  int ret = config_set_userinfo(userinfo->lockout_threshold,
                                userinfo->lockout_duration,
                                userinfo->session_timeout);
  if (ret < 0)
    {
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->result = IPC_OK;
  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_set_account(pipeline_node_t *node,
                                                   int event_id,
                                                   void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  if (!ctx->req || ctx->req->data_len < sizeof(ipc_config_account_info_t))
    {
      ctx->result = IPC_ERROR_INVALID_PARAM;
      return NODE_FAILED;
    }

  ipc_config_account_info_t *account = (ipc_config_account_info_t *)ctx->req->data;
  int ret = config_update_account_state(account->username, account->login_status);
  if (ret < 0)
    {
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->result = IPC_OK;
  return NODE_SUCCESS;
}

static node_result_t ipc_config_node_save(pipeline_node_t *node,
                                            int event_id,
                                            void *event_data)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)node->ctx;

  FAR void *json_obj = config_load();
  if (!json_obj)
    {
      ctx->result = IPC_ERROR_NOT_FOUND;
      return NODE_FAILED;
    }

  int ret = config_save(json_obj);
  if (ret < 0)
    {
      ctx->result = ret;
      return NODE_FAILED;
    }

  ctx->result = IPC_OK;
  return NODE_SUCCESS;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void ipc_handle_config(ipc_request_t *req, ipc_client_t *client)
{
  ipc_config_ctx_t *ctx = (ipc_config_ctx_t *)malloc(sizeof(ipc_config_ctx_t));
  if (!ctx)
    {
      IPC_ERR("failed to allocate ctx");
      ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_NO_MEMORY));
      return;
    }

  ctx->client = client;
  ctx->result = IPC_ERROR_INVALID_MSG;
  ctx->req = NULL;
  ctx->resp_data = NULL;
  ctx->resp_data_len = 0;

  if (req && req->data_len > 0)
    {
      ctx->req = (ipc_request_t *)malloc(sizeof(ipc_request_t) + req->data_len);
      if (!ctx->req)
        {
          IPC_ERR("failed to allocate req");
          ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_NO_MEMORY));
          free(ctx);
          return;
        }
      memcpy(ctx->req, req, sizeof(ipc_request_t) + req->data_len);
    }

  uint32_t msg_id = req ? req->msg_id : 0;
  pipeline_t *p = pipeline_create("ipc_config", 5000,
                                 ipc_config_pipeline_complete, ctx);
  if (!p)
    {
      IPC_ERR("failed to create pipeline");
      ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_NO_MEMORY));
      ipc_config_ctx_free(ctx);
      return;
    }

  switch (msg_id)
    {
      case IPC_CONFIG_GET_VERSION:
        pipeline_add_node(p, "get_version",
                         ipc_config_node_get_version, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_GET_WIFI_STA:
        pipeline_add_node(p, "get_wifi_sta",
                         ipc_config_node_get_wifi_sta, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_GET_WIFI_AP:
        pipeline_add_node(p, "get_wifi_ap",
                         ipc_config_node_get_wifi_ap, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_GET_MAX_USERS:
        pipeline_add_node(p, "get_max_users",
                         ipc_config_node_get_max_users, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_GET_USERINFO:
        pipeline_add_node(p, "get_userinfo",
                         ipc_config_node_get_userinfo, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_GET_ACCOUNT:
        pipeline_add_node(p, "get_account",
                         ipc_config_node_get_account, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_SET_WIFI_STA:
        pipeline_add_node(p, "set_wifi_sta",
                         ipc_config_node_set_wifi_sta, ctx, NO_WAIT_EVENT);
        pipeline_add_node(p, "save",
                         ipc_config_node_save, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_SET_WIFI_AP:
        pipeline_add_node(p, "set_wifi_ap",
                         ipc_config_node_set_wifi_ap, ctx, NO_WAIT_EVENT);
        pipeline_add_node(p, "save",
                         ipc_config_node_save, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_SET_USERINFO:
        pipeline_add_node(p, "set_userinfo",
                         ipc_config_node_set_userinfo, ctx, NO_WAIT_EVENT);
        pipeline_add_node(p, "save",
                         ipc_config_node_save, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_SET_ACCOUNT:
        pipeline_add_node(p, "set_account",
                         ipc_config_node_set_account, ctx, NO_WAIT_EVENT);
        pipeline_add_node(p, "save",
                         ipc_config_node_save, ctx, NO_WAIT_EVENT);
        break;

      case IPC_CONFIG_GET:
      case IPC_CONFIG_SET:
      case IPC_CONFIG_RELOAD:
      default:
        ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_INVALID_MSG));
        pipeline_destroy_async(p);
        ipc_config_ctx_free(ctx);
        return;
    }

  pipeline_start(p);
}
