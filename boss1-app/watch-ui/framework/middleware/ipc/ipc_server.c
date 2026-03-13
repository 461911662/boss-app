/****************************************************************************
 * apps/watch-ui/framework/middleware/ipc/ipc_server.c
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
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "ipc/ipc_proto.h"
#include "ipc/ipc_server.h"
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

#ifndef CONFIG_IPC_MAX_CLIENTS
#  define CONFIG_IPC_MAX_CLIENTS     4
#endif

#define IPC_SERVER_THREAD_STACKSIZE 4096
#define IPC_SERVER_THREAD_PRIORITY 100

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct ipc_client_s
{
  int fd;
  char *recv_buf;
  size_t recv_offset;
  size_t recv_capacity;
  struct ipc_client_s *next;
  void (*cplt_cbk)(struct ipc_client_s *client);
} ipc_client_t;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void *ipc_server_thread(FAR void *arg);
static FAR ipc_client_t *ipc_client_list_remove(FAR ipc_client_t *client);
static void ipc_client_free(FAR ipc_client_t *client);
static void ipc_close_client(FAR ipc_client_t *client);
static int ipc_verify_packet_ok(FAR ipc_client_t *client);
static int ipc_process_client(FAR ipc_client_t *client);
static void ipc_collect_fds(FAR fd_set *readfds, int *maxfd);
static void ipc_handle_new_connection(FAR fd_set *readfds);
static void ipc_handle_client_messages(FAR fd_set *readfds);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static ipc_client_t *g_clients = NULL;
static pthread_mutex_t g_clients_lock;
static ipc_client_t *g_pending_clients = NULL;
static pthread_mutex_t g_pending_lock;
static pthread_t g_server_thread;
static int g_server_fd = -1;
static bool g_server_running = false;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ipc_client_close_cplt
 *
 * Description:
 *   Completion callback for closing client connection after response sent.
 *
 ****************************************************************************/

static void ipc_client_close_cplt(FAR ipc_client_t *client)
{
  if (client)
    {
      IPC_LOG("ipc_client_close_cplt: closing client fd=%d", client->fd);
      ipc_client_free(client);
    }
}

/****************************************************************************
 * Name: ipc_client_new
 *
 * Description:
 *   Create a new client structure.
 *
 * Input Parameters:
 *   fd - Client socket file descriptor.
 *
 * Returned Value:
 *   Pointer to new client structure, or NULL on failure.
 *
 ****************************************************************************/

static FAR ipc_client_t *ipc_client_new(int fd)
{
  FAR ipc_client_t *client = (FAR ipc_client_t *)calloc(1, sizeof(ipc_client_t));
  if (client)
    {
      client->fd = fd;
    }
  return client;
}

/****************************************************************************
 * Name: ipc_client_free
 *
 * Description:
 *   Free client structure and associated resources.
 *
 ****************************************************************************/

static void ipc_client_free(FAR ipc_client_t *client)
{
  if (!client)
    {
      return;
    }

  if (client->fd >= 0)
    {
      close(client->fd);
    }
  if (client->recv_buf)
    {
      free(client->recv_buf);
    }

  free(client);
}

/****************************************************************************
 * Name: ipc_close_client
 *
 * Description:
 *   Remove client from list and free resources.
 *
 ****************************************************************************/

static void ipc_close_client(FAR ipc_client_t *client)
{
  if (client)
    {
      FAR ipc_client_t *to_close = ipc_client_list_remove(client);
      if (to_close)
        {
          ipc_client_free(to_close);
        }
    }
}

/****************************************************************************
 * Name: ipc_client_list_add
 *
 * Description:
 *   Add client to the beginning of the client list.
 *
 ****************************************************************************/

static void ipc_client_list_add(FAR ipc_client_t *client)
{
  pthread_mutex_lock(&g_clients_lock);
  client->next = g_clients;
  g_clients = client;
  pthread_mutex_unlock(&g_clients_lock);
}

/****************************************************************************
 * Name: ipc_client_list_remove
 *
 * Description:
 *   Remove client from the client list.
 *   Note: Must be called with g_clients_lock held.
 *
 * Returned Value:
 *   Pointer to removed client, or NULL if not found.
 *
 ****************************************************************************/

static FAR ipc_client_t *ipc_client_list_remove(FAR ipc_client_t *client)
{
  FAR ipc_client_t *ret = NULL;
  FAR ipc_client_t *prev = NULL;
  FAR ipc_client_t *curr = g_clients;

  while (curr)
    {
      if (curr == client)
        {
          ret = curr;
          if (prev)
            {
              prev->next = curr->next;
            }
          else
            {
              g_clients = curr->next;
            }
          break;
        }
      prev = curr;
      curr = curr->next;
    }

  return ret;
}

/****************************************************************************
 * Name: ipc_pending_list_add
 *
 * Description:
 *   Add client to the pending list for external processing.
 *
 ****************************************************************************/

static void ipc_pending_list_add(FAR ipc_client_t *client)
{
  if (!client)
    {
      return;
    }

  pthread_mutex_lock(&g_pending_lock);

  client->next = NULL;
  if (g_pending_clients == NULL)
    {
      g_pending_clients = client;
    }
  else
    {
      FAR ipc_client_t *tail = g_pending_clients;
      while (tail->next)
        {
          tail = tail->next;
        }
      tail->next = client;
    }

  pthread_mutex_unlock(&g_pending_lock);
}

/****************************************************************************
 * Name: ipc_server_get_pending_clients
 *
 * Description:
 *   Get and remove all pending clients from the list.
 *
 * Returned Value:
 *   Pointer to the head of pending client list, or NULL if empty.
 *   The list is cleared after this call.
 *
 ****************************************************************************/

static FAR ipc_client_t *ipc_server_get_pending_clients(void)
{
  FAR ipc_client_t *pending = NULL;

  pthread_mutex_lock(&g_pending_lock);
  pending = g_pending_clients;
  if (g_pending_clients)
    {
      g_pending_clients = g_pending_clients->next;
    }
  pthread_mutex_unlock(&g_pending_lock);

  return pending;
}

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

ipc_response_t *ipc_create_error_resp(int error)
{
  FAR ipc_response_t *resp = (FAR ipc_response_t *)malloc(sizeof(ipc_response_t));
  if (resp)
    {
      memset(resp, 0, sizeof(ipc_response_t));
      resp->magic = IPC_MAGIC;
      resp->result = error;
      resp->data_len = 0;
    }
  return resp;
}

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

ipc_response_t *ipc_create_success_resp(FAR const void *data, size_t data_len)
{
  FAR ipc_response_t *resp = (FAR ipc_response_t *)malloc(sizeof(ipc_response_t) + data_len);
  if (resp)
    {
      memset(resp, 0, sizeof(ipc_response_t));
      resp->magic = IPC_MAGIC;
      resp->result = IPC_OK;
      resp->data_len = data_len;
      if (data && data_len > 0)
        {
          memcpy(resp->data, data, data_len);
        }
    }
  return resp;
}

/****************************************************************************
 * Name: ipc_send_response
 *
 * Description:
 *   Send response to client and call completion callback.
 *
 ****************************************************************************/

void ipc_send_response(FAR ipc_client_t *client, FAR ipc_response_t *resp)
{
  if (!client || !resp || client->fd < 0)
    {
      if (resp)
        {
          free(resp);
        }
      return;
    }

  resp->checksum = ipc_calculate_checksum(resp->data, resp->data_len);

  size_t send_len = sizeof(ipc_response_t) + resp->data_len;
  ssize_t nwritten = send(client->fd, resp, send_len, 0);
  if (nwritten <= 0)
    {
      IPC_ERR("send failed");
    }

  free(resp);

  /* Call completion callback to close client connection */
  if (client->cplt_cbk)
    {
      client->cplt_cbk(client);
    }
}

/****************************************************************************
 * Name: ipc_handle_request
 *
 * Description:
 *   Route IPC request to appropriate handler based on message type.
 *
 ****************************************************************************/

static void ipc_handle_request(FAR ipc_request_t *req, FAR ipc_client_t *client)
{
  uint16_t msg_type;

  if (!req)
    {
      FAR ipc_response_t *resp = ipc_create_error_resp(IPC_ERROR_INVALID_PARAM);
      ipc_send_response(client, resp);
      return;
    }

  msg_type = IPC_GET_MSG_TYPE(req->msg_id);

  switch (msg_type)
    {
      case IPC_TYPE_CONFIG:
        ipc_handle_config(req, client);
        break;

      case IPC_TYPE_APWIFI:
        /* ipc_handle_apwifi(req, client); */
        break;

      case IPC_TYPE_STAWIFI:
        {
          IPC_ERR("ipc_handle_request: unknown msg_type=%d", msg_type);
          FAR ipc_response_t *resp = ipc_create_error_resp(IPC_ERROR_INVALID_MSG);
          ipc_send_response(client, resp);
        }
        break;
    }
}

/****************************************************************************
 * Name: ipc_verify_packet_ok
 *
 * Description:
 *   Verify if received data forms a complete and valid IPC packet.
 *
 * Returned Value:
 *   0   - Packet is valid and complete
 *   -1  - Packet is incomplete, need more data
 *   -2  - Packet is invalid
 *
 ****************************************************************************/

static int ipc_verify_packet_ok(FAR ipc_client_t *client)
{
  if (!client || client->fd < 0)
    {
      return -2;
    }

  if (client->recv_offset >= sizeof(ipc_request_t))
    {
      FAR ipc_request_t *req = (FAR ipc_request_t *)client->recv_buf;
      if (req->magic != IPC_MAGIC)
        {
          IPC_ERR("ipc_verify_packet_ok: invalid magic");
          return -2;
        }

      if (req->data_len > CONFIG_IPC_MAX_REQUEST_LEN - sizeof(ipc_request_t))
        {
          IPC_ERR("ipc_verify_packet_ok: data_len too large");
          return -2;
        }

      size_t total_len = sizeof(ipc_request_t) + req->data_len;
      if (client->recv_offset < total_len)
        {
          return -1;
        }

      if (!ipc_verify_request_checksum(req))
        {
          IPC_ERR("ipc_verify_packet_ok: invalid checksum");
          return -2;
        }
    }
  else
    {
      return -1;
    }

  return 0;
}

/****************************************************************************
 * Name: ipc_process_client
 *
 * Description:
 *   Process IPC request from client.
 *
 ****************************************************************************/

static int ipc_process_client(FAR ipc_client_t *client)
{
  FAR ipc_request_t *req;

  if (!client || client->fd < 0)
    {
      return -1;
    }

  req = (FAR ipc_request_t *)client->recv_buf;

  /* Set completion callback to close client after response sent */
  client->cplt_cbk = ipc_client_close_cplt;
  ipc_handle_request(req, client);

  return 0;
}

/****************************************************************************
 * Name: ipc_collect_fds
 *
 * Description:
 *   Collect all file descriptors to monitor with select().
 *
 ****************************************************************************/

static void ipc_collect_fds(FAR fd_set *readfds, int *maxfd)
{
  int fd_count = 0;
  int fds[CONFIG_IPC_MAX_CLIENTS + 1];

  FD_ZERO(readfds);
  *maxfd = g_server_fd;
  fds[fd_count++] = g_server_fd;

  pthread_mutex_lock(&g_clients_lock);
  FAR ipc_client_t *client = g_clients;
  while (client && fd_count < CONFIG_IPC_MAX_CLIENTS + 1)
    {
      fds[fd_count++] = client->fd;
      if (client->fd > *maxfd)
        {
          *maxfd = client->fd;
        }
      client = client->next;
    }
  pthread_mutex_unlock(&g_clients_lock);

  for (int i = 0; i < fd_count; i++)
    {
      FD_SET(fds[i], readfds);
    }
}

/****************************************************************************
 * Name: ipc_handle_new_connection
 *
 * Description:
 *   Accept new client connections.
 *
 ****************************************************************************/

static void ipc_handle_new_connection(FAR fd_set *readfds)
{
  if (!FD_ISSET(g_server_fd, readfds))
    {
      return;
    }

  struct sockaddr_un client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd = accept(g_server_fd, (FAR struct sockaddr *)&client_addr, &client_len);

  if (client_fd >= 0)
    {
      FAR ipc_client_t *new_client = ipc_client_new(client_fd);
      if (new_client)
        {
          ipc_client_list_add(new_client);
        }
      else
        {
          close(client_fd);
        }
    }
}

/****************************************************************************
 * Name: ipc_handle_client_messages
 *
 * Description:
 *   Handle incoming data from connected clients.
 *
 ****************************************************************************/

static void ipc_handle_client_messages(FAR fd_set *readfds)
{
  int ret;
  char buf[56];
  FAR ipc_client_t *next = NULL;
  FAR ipc_client_t *client;

  pthread_mutex_lock(&g_clients_lock);
  client = g_clients;

  while (client)
    {
      next = client->next;
      if (FD_ISSET(client->fd, readfds))
        {
          ssize_t nread = recv(client->fd, buf, sizeof(buf), 0);

          if (nread <= 0)
            {
              ipc_close_client(client);
              client = next;
              continue;
            }

          if (client->recv_offset + nread > client->recv_capacity)
            {
              FAR char *new_buf;
              client->recv_capacity = client->recv_offset + nread + 50;
              new_buf = realloc(client->recv_buf, client->recv_capacity);
              if (new_buf == NULL)
                {
                  IPC_ERR("realloc failed");
                  ipc_close_client(client);
                  client = next;
                  continue;
                }
              client->recv_buf = new_buf;
            }

          memcpy(client->recv_buf + client->recv_offset, buf, nread);
          client->recv_offset += nread;

          ret = ipc_verify_packet_ok(client);
          if (ret == 0)
            {
              /* Add to pending list for external processing */
              ipc_pending_list_add(ipc_client_list_remove(client));
            }
          else if (ret == -2)
            {
              /* Invalid packet, close client */
              ipc_close_client(client);
            }
          /* ret == -1: incomplete packet, continue waiting */
        }
      client = next;
    }
  pthread_mutex_unlock(&g_clients_lock);
}

/****************************************************************************
 * Name: ipc_server_thread
 *
 * Description:
 *   Main server thread that handles accept and I/O events.
 *
 ****************************************************************************/

static FAR void *ipc_server_thread(FAR void *arg)
{
  (void)arg;

  IPC_LOG("ipc_server_thread: started");

  while (g_server_running)
    {
      fd_set readfds;
      int maxfd;
      ipc_collect_fds(&readfds, &maxfd);

      struct timeval tv = {1, 0};
      int ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);

      if (ret <= 0)
        {
          continue;
        }

      if (FD_ISSET(g_server_fd, &readfds))
        {
          ipc_handle_new_connection(&readfds);
        }
      else
        {
          ipc_handle_client_messages(&readfds);
        }
    }

  IPC_LOG("ipc_server_thread: stopped");
  return NULL;
}

/****************************************************************************
 * Public Functions
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

int ipc_server_init(void)
{
  int ret;

  IPC_LOG("ipc_server_init: starting...");

  /* Initialize client list lock */
  ret = pthread_mutex_init(&g_clients_lock, NULL);
  if (ret != 0)
    {
      IPC_ERR("ipc_server_init: pthread_mutex_init failed");
      return -1;
    }
  g_clients = NULL;

  /* Initialize pending list lock */
  ret = pthread_mutex_init(&g_pending_lock, NULL);
  if (ret != 0)
    {
      IPC_ERR("ipc_server_init: pthread_mutex_init g_pending_lock failed");
      pthread_mutex_destroy(&g_clients_lock);
      return -1;
    }
  g_pending_clients = NULL;

  /* Create server socket */
  g_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (g_server_fd < 0)
    {
      IPC_ERR("ipc_server_init: socket failed");
      goto cleanup_mutex;
    }

  /* Create directory and bind socket */
  (void)mkdir(CONFIG_NET_LOCAL_VFS_PATH, 0777);
  (void)unlink(CONFIG_IPC_SOCKET_PATH);

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, CONFIG_IPC_SOCKET_PATH, sizeof(addr.sun_path) - 1);

  ret = bind(g_server_fd, (FAR struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0)
    {
      IPC_ERR("ipc_server_init: bind failed: %s", strerror(errno));
      goto cleanup_fd;
    }

  /* Listen for connections */
  ret = listen(g_server_fd, CONFIG_IPC_MAX_CLIENTS);
  if (ret < 0)
    {
      IPC_ERR("ipc_server_init: listen failed");
      goto cleanup_fd;
    }

  IPC_LOG("ipc_server_init: listening on %s", CONFIG_IPC_SOCKET_PATH);

  /* Start server thread */
  g_server_running = true;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, IPC_SERVER_THREAD_STACKSIZE);

  ret = pthread_create(&g_server_thread, &attr, ipc_server_thread, NULL);
  pthread_attr_destroy(&attr);

  if (ret < 0)
    {
      IPC_ERR("ipc_server_init: pthread_create failed");
      goto cleanup_fd;
    }

  IPC_LOG("ipc_server_init: success");
  return 0;

cleanup_fd:
  close(g_server_fd);
  g_server_fd = -1;

cleanup_mutex:
  pthread_mutex_destroy(&g_clients_lock);
  pthread_mutex_destroy(&g_pending_lock);

  g_server_running = false;

  return -1;
}

/****************************************************************************
 * Name: ipc_server_deinit
 *
 * Description:
 *   Stop server and release all resources.
 *
 ****************************************************************************/

void ipc_server_deinit(void)
{
  IPC_LOG("ipc_server_deinit: stopping...");

  g_server_running = false;

  /* Wait for server thread to exit */
  if (g_server_thread)
    {
      pthread_join(g_server_thread, NULL);
    }

  /* Close server socket */
  if (g_server_fd >= 0)
    {
      close(g_server_fd);
      g_server_fd = -1;
    }

  (void)unlink(CONFIG_IPC_SOCKET_PATH);

  /* Close all connected clients */
  pthread_mutex_lock(&g_clients_lock);
  FAR ipc_client_t *client = g_clients;
  while (client)
    {
      FAR ipc_client_t *next = client->next;
      ipc_client_free(client);
      client = next;
    }
  g_clients = NULL;
  pthread_mutex_unlock(&g_clients_lock);
  pthread_mutex_destroy(&g_clients_lock);

  /* Close all pending clients */
  pthread_mutex_lock(&g_pending_lock);
  FAR ipc_client_t *pending = g_pending_clients;
  while (pending)
    {
      FAR ipc_client_t *next = pending->next;
      ipc_client_free(pending);
      pending = next;
    }
  g_pending_clients = NULL;
  pthread_mutex_unlock(&g_pending_lock);
  pthread_mutex_destroy(&g_pending_lock);

  IPC_LOG("ipc_server_deinit: done");
}

/****************************************************************************
 * Name: ipc_server_poll
 *
 * Description:
 *   Poll and process pending client requests.
 *   Should be called periodically from main loop.
 *
 ****************************************************************************/

void ipc_server_poll(void)
{
  FAR ipc_client_t *client;

  if (!g_server_running)
    {
      return;
    }

  while ((client = ipc_server_get_pending_clients()) != NULL)
    {
      ipc_process_client(client);
    }
}
