/****************************************************************************
 * apps/watch-ui/ipc/ipc_server/ipc_handler_apwifi.c
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

#include <uv.h>

#include "middleware/pipeline/pipeline.h"
#include "../ipc_proto.h"
#include "ipc_server.h"
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
 * Private Types
 ****************************************************************************/

typedef struct
{
  ipc_request_t *req;
  ipc_client_t *client;
  uint32_t msg_id;
} ipc_apwifi_ctx_t;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void ipc_apwifi_pipeline_complete(pipeline_t *p, bool success, void *user_data)
{
  ipc_apwifi_ctx_t *ctx = (ipc_apwifi_ctx_t *)user_data;
  ipc_response_t *resp = NULL;

  if (success)
    {
      switch (ctx->msg_id)
        {
          case IPC_APWIFI_GET_SSID:
            resp = ipc_create_success_resp("ap_ssid", 7);
            break;

          case IPC_APWIFI_SET_SSID:
            resp = ipc_create_success_resp(NULL, 0);
            break;

          default:
            resp = ipc_create_error_resp(IPC_ERROR_INVALID_MSG);
            break;
        }
    }
  else
    {
      resp = ipc_create_error_resp(IPC_ERROR_INVALID_MSG);
    }

  if (ctx->client)
    {
      ipc_send_response(ctx->client, resp);
    }

  if (ctx->req)
    {
      free(ctx->req);
    }
  free(ctx);
  pipeline_destroy_async(p);
}

static node_result_t ipc_apwifi_node_process(pipeline_node_t *node, int event_id, void *event_data)
{
  ipc_apwifi_ctx_t *ctx = (ipc_apwifi_ctx_t *)node->ctx;

  switch (ctx->msg_id)
    {
      case IPC_APWIFI_GET_SSID:
        break;

      case IPC_APWIFI_SET_SSID:
        break;

      default:
        return NODE_FAILED;
    }

  return NODE_SUCCESS;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void ipc_handle_apwifi(ipc_request_t *req, ipc_client_t *client)
{
  uint32_t msg_id = req ? req->msg_id : 0;

  ipc_apwifi_ctx_t *ctx = (ipc_apwifi_ctx_t *)malloc(sizeof(ipc_apwifi_ctx_t));
  if (!ctx)
    {
      ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_NO_MEMORY));
      return;
    }

  ctx->req = req;
  ctx->client = client;
  ctx->msg_id = msg_id;

  pipeline_t *p = pipeline_create("ipc_apwifi", 5000, ipc_apwifi_pipeline_complete, ctx);
  if (!p)
    {
      free(ctx);
      ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_NO_MEMORY));
      return;
    }

  pipeline_add_node(p, "apwifi_process", ipc_apwifi_node_process, ctx, NO_WAIT_EVENT);

  int ret = pipeline_start(p);
  if (ret < 0)
    {
      pipeline_destroy_async(p);
      free(ctx);
      ipc_send_response(client, ipc_create_error_resp(IPC_ERROR_INVALID_MSG));
      return;
    }
}
