/****************************************************************************
 * apps/watch-ui/framework/middleware/ipc/ipc_server.h
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

#ifndef __APPS_WATCH_UI_IPC_IPC_SERVER_H
#define __APPS_WATCH_UI_IPC_IPC_SERVER_H

#include <nuttx/config.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ipc/ipc_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Forward Declarations
 ****************************************************************************/

struct ipc_client_s;
typedef struct ipc_client_s ipc_client_t;

/****************************************************************************
 * IPC Server Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ipc_server_init
 *
 * Description:
 *   Initialize IPC server and start server thread.
 *
 * Returned Value:
 *   0 on success, -1 on failure.
 *
 ****************************************************************************/

int ipc_server_init(void);

/****************************************************************************
 * Name: ipc_server_deinit
 *
 * Description:
 *   Stop server and release all resources.
 *
 ****************************************************************************/

void ipc_server_deinit(void);

/****************************************************************************
 * Name: ipc_server_poll
 *
 * Description:
 *   Poll and process pending client requests.
 *   Should be called periodically from main loop.
 *
 ****************************************************************************/

void ipc_server_poll(void);

/****************************************************************************
 * Response Helper Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ipc_create_error_resp
 *
 * Description:
 *   Create an error response packet.
 *
 * Input Parameters:
 *   error - Error code.
 *
 * Returned Value:
 *   Pointer to response structure, or NULL on failure.
 *
 ****************************************************************************/

ipc_response_t *ipc_create_error_resp(int error);

/****************************************************************************
 * Name: ipc_create_success_resp
 *
 * Description:
 *   Create a success response packet with data.
 *
 * Input Parameters:
 *   data     - Pointer to response data.
 *   data_len - Length of response data.
 *
 * Returned Value:
 *   Pointer to response structure, or NULL on failure.
 *
 ****************************************************************************/

ipc_response_t *ipc_create_success_resp(FAR const void *data, size_t data_len);

/****************************************************************************
 * Name: ipc_send_response
 *
 * Description:
 *   Send response to client and call completion callback.
 *
 ****************************************************************************/

void ipc_send_response(FAR ipc_client_t *client, FAR ipc_response_t *resp);

/****************************************************************************
 * Request Handlers
 ****************************************************************************/

/****************************************************************************
 * Name: ipc_handle_config
 *
 * Description:
 *   Handle IPC configuration request.
 *
 ****************************************************************************/

void ipc_handle_config(FAR ipc_request_t *req, FAR ipc_client_t *client);

/****************************************************************************
 * Name: ipc_handle_apwifi
 *
 * Description:
 *   Handle IPC AP WiFi request.
 *
 ****************************************************************************/

void ipc_handle_apwifi(FAR ipc_request_t *req, FAR ipc_client_t *client);

/****************************************************************************
 * Name: ipc_handle_stawifi
 *
 * Description:
 *   Handle IPC STA WiFi request.
 *
 ****************************************************************************/

void ipc_handle_stawifi(FAR ipc_request_t *req, FAR ipc_client_t *client);

#ifdef __cplusplus
}
#endif

#endif
