/****************************************************************************
 * apps/testing/ipctest/ipctest_main.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <watchui/log.h>
#include "ipc/ipc_client.h"

#define IPC_TEST_LOG(fmt, ...)    appinfo("[IPCTest] " fmt, ##__VA_ARGS__)
#define IPC_TEST_ERR(fmt, ...)   apperr("[IPCTest] " fmt, ##__VA_ARGS__)

#ifdef CONFIG_TESTING_IPCTEST

static int test_get_version(void)
{
  int ret;
  uint32_t version;

  IPC_TEST_LOG("Test 1: Get Config Version");
  ret = ipc_config_get_version(&version);
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Version: %u", version);
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_get_wifi_sta(void)
{
  int ret;
  char ssid[64];
  char password[64];

  IPC_TEST_LOG("Test 2: Get WiFi STA");
  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
  ret = ipc_config_get_sta(ssid, password, sizeof(ssid));
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  SSID: %s", ssid);
      IPC_TEST_LOG("  Password: %s", password);
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_get_wifi_ap(void)
{
  int ret;
  char ssid[64];

  IPC_TEST_LOG("Test 3: Get WiFi AP");
  memset(ssid, 0, sizeof(ssid));
  ret = ipc_config_get_ap(ssid, sizeof(ssid));
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  AP SSID: %s", ssid);
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_get_max_users(void)
{
  int ret;
  uint32_t max_users;

  IPC_TEST_LOG("Test 4: Get Max Users");
  ret = ipc_config_get_max_users(&max_users);
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Max Users: %u", max_users);
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_get_userinfo(void)
{
  int ret;
  int lockout_threshold;
  int lockout_duration;
  int session_timeout;

  IPC_TEST_LOG("Test 5: Get Userinfo");
  ret = ipc_config_get_userinfo(&lockout_threshold, &lockout_duration, &session_timeout);
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Lockout Threshold: %d", lockout_threshold);
      IPC_TEST_LOG("  Lockout Duration: %d", lockout_duration);
      IPC_TEST_LOG("  Session Timeout: %d", session_timeout);
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_set_wifi_sta(void)
{
  int ret;

  IPC_TEST_LOG("Test 6: Set WiFi STA");
  ret = ipc_config_set_sta("TestSSID", "TestPassword123");
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Set STA success");
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_set_wifi_ap(void)
{
  int ret;

  IPC_TEST_LOG("Test 7: Set WiFi AP");
  ret = ipc_config_set_ap("TestAP");
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Set AP success");
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_set_userinfo(void)
{
  int ret;

  IPC_TEST_LOG("Test 8: Set Userinfo");
  ret = ipc_config_set_userinfo(3, 180, 1800);
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Set Userinfo success");
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

static int test_get_account(void)
{
  int ret;
  int account_status;

  IPC_TEST_LOG("Test 9: Get Account");
  ret = ipc_config_get_account("admin", &account_status);
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Account Status: %d", account_status);
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return 0;
}

static int test_set_account(void)
{
  int ret;

  IPC_TEST_LOG("Test 10: Set Account");
  ret = ipc_config_set_account("testuser", 1);
  if (ret == IPC_OK)
    {
      IPC_TEST_LOG("  Set Account success");
      return 0;
    }

  IPC_TEST_ERR("  Failed: %d", ret);
  return -1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int ret;

  IPC_TEST_LOG("Starting IPC Test...");

  ret = test_get_version();
  if (ret < 0) return ret;

  ret = test_get_wifi_sta();
  if (ret < 0) return ret;

  ret = test_get_wifi_ap();
  if (ret < 0) return ret;

  ret = test_get_max_users();
  if (ret < 0) return ret;

  ret = test_get_userinfo();
  if (ret < 0) return ret;

  ret = test_set_wifi_sta();
  if (ret < 0) return ret;

  ret = test_set_wifi_ap();
  if (ret < 0) return ret;

  ret = test_set_userinfo();
  if (ret < 0) return ret;

  ret = test_get_account();
  if (ret < 0) return ret;

  ret = test_set_account();
  if (ret < 0) return ret;

  IPC_TEST_LOG("IPC Test Complete!");
  return 0;
}

#endif /* CONFIG_TESTING_IPCTEST */
