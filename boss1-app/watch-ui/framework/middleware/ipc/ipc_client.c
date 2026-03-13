/****************************************************************************
 * apps/watch-ui/ipc/ipc_client.c
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>

#include "ipc/ipc_proto.h"
#include "ipc/ipc_client.h"
#include <watchui/log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_IPC_DEBUG
#  define IPC_LOG(fmt, ...)    appinfo("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_DBG(fmt, ...)   appdbg("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_WARN(fmt, ...)  appwarn("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_ERR(fmt, ...)   apperr("[IPC] " fmt, ##__VA_ARGS__)
#else
#  define IPC_LOG(fmt, ...)
#  define IPC_DBG(fmt, ...)
#  define IPC_WARN(fmt, ...)  appwarn("[IPC] " fmt, ##__VA_ARGS__)
#  define IPC_ERR(fmt, ...)   apperr("[IPC] " fmt, ##__VA_ARGS__)
#endif

#ifndef CONFIG_IPC_TIMEOUT_S
#  define CONFIG_IPC_TIMEOUT_S 3
#endif

#define IPC_TIMEOUT_S CONFIG_IPC_TIMEOUT_S

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static ipc_response_t *ipc_do_request(FAR ipc_request_t *req)
{
  if (!req || req->magic != IPC_MAGIC)
    {
      IPC_ERR("ipc_do_request: invalid request");
      return NULL;
    }

  req->checksum = ipc_calculate_checksum(req->data, req->data_len);

  struct sockaddr_un addr;
  ssize_t nread;
  ipc_response_t resp_header;
  ipc_response_t *resp = NULL;

  size_t send_len = sizeof(ipc_request_t) + req->data_len;
  if (send_len > CONFIG_IPC_MAX_REQUEST_LEN)
    {
      IPC_ERR("ipc_do_request: request too large");
      return NULL;
    }

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    {
      IPC_ERR("socket() failed");
      return NULL;
    }

  struct timeval timeout;
  timeout.tv_sec = IPC_TIMEOUT_S;
  timeout.tv_usec = 0;
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
      IPC_ERR("setsockopt RCVTIMEO failed");
      close(fd);
      return NULL;
    }

  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    {
      IPC_ERR("setsockopt SNDTIMEO failed");
      close(fd);
      return NULL;
    }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, CONFIG_IPC_SOCKET_PATH, sizeof(addr.sun_path) - 1);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      IPC_ERR("connect() failed: %s", strerror(errno));
      close(fd);
      return NULL;
    }

  ssize_t nwritten = send(fd, req, send_len, 0);
  if (nwritten <= 0)
    {
      IPC_ERR("send() failed: %zd", nwritten);
      close(fd);
      return NULL;
    }

  size_t resp_header_size = sizeof(ipc_response_t);
  size_t total_recv = recv(fd, &resp_header, resp_header_size, 0);
  if (total_recv != resp_header_size)
    {
      IPC_ERR("recv() header failed: %zu (expected %zu)", total_recv, resp_header_size);
      close(fd);
      return NULL;
    }

  if (resp_header.magic != IPC_MAGIC)
    {
      IPC_ERR("invalid magic: 0x%08x", resp_header.magic);
      close(fd);
      return NULL;
    }

  if (resp_header.data_len > CONFIG_IPC_RESPONSE_MAX_DATA_LEN)
    {
      IPC_ERR("data_len too large: %u", resp_header.data_len);
      close(fd);
      return NULL;
    }

  size_t resp_len = resp_header_size + resp_header.data_len;
  resp = (ipc_response_t *)malloc(resp_len);
  if (!resp)
    {
      IPC_ERR("ipc_do_request: failed to allocate response");
      close(fd);
      return NULL;
    }

  memcpy(resp, &resp_header, resp_header_size);

  while (total_recv < resp_len)
    {
      nread = recv(fd, (char *)resp + total_recv, resp_len - total_recv, 0);
      if (nread <= 0)
        {
          free(resp);
          close(fd);
          return NULL;
        }
      total_recv += nread;
    }

  close(fd);

  if (!ipc_verify_response_checksum(resp))
    {
      free(resp);
      return NULL;
    }

  return resp;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

ipc_response_t *ipc_send_request(FAR ipc_request_t *req)
{
  return ipc_do_request(req);
}
