/****************************************************************************
 * apps/testing/passwdtest/passwdtest_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
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
#include <errno.h>

#include <fsutils/passwd.h>

#define PASSWD_TEST_LOG(fmt, ...)    printf("[PasswdTest] " fmt "\n", ##__VA_ARGS__)
#define PASSWD_TEST_ERR(fmt, ...)   printf("[PasswdTest] ERROR: " fmt "\n", ##__VA_ARGS__)

#ifdef CONFIG_TESTING_PASSWDTEST

static int test_passwd_add_user(void)
{
  int ret;

  PASSWD_TEST_LOG("Test 1: Add User");

  ret = passwd_adduser("testuser", "testpass123");
  if (ret != 0)
    {
      PASSWD_TEST_ERR("  passwd_adduser failed: %d", ret);
      return -1;
    }

  PASSWD_TEST_LOG("  User added successfully");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

static int test_passwd_verify_user(void)
{
  int ret;

  PASSWD_TEST_LOG("Test 2: Verify User");

  ret = passwd_verify("testuser", "testpass123");
  if (ret != 1)
    {
      PASSWD_TEST_ERR("  passwd_verify failed: %d (expected 1)", ret);
      return -1;
    }

  PASSWD_TEST_LOG("  User verified successfully");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

static int test_passwd_verify_wrong_password(void)
{
  int ret;

  PASSWD_TEST_LOG("Test 3: Verify Wrong Password");

  ret = passwd_verify("testuser", "wrongpassword");
  if (ret == 1)
    {
      PASSWD_TEST_ERR("  Expected mismatch, but passed");
      return -1;
    }

  PASSWD_TEST_LOG("  Correctly rejected wrong password");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

static int test_passwd_find_nonexistent(void)
{
  bool found;

  PASSWD_TEST_LOG("Test 4: Find Non-existent User");

  found = passwd_finduser("nonexistent");
  if (found)
    {
      PASSWD_TEST_ERR("  Expected false, got true");
      return -1;
    }

  PASSWD_TEST_LOG("  Correctly returned false");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

static int test_passwd_find_existing(void)
{
  bool found;

  PASSWD_TEST_LOG("Test 5: Find Existing User");

  found = passwd_finduser("testuser");
  if (!found)
    {
      PASSWD_TEST_ERR("  Expected true, got false");
      return -1;
    }

  PASSWD_TEST_LOG("  Correctly returned true");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

static int test_passwd_del_user(void)
{
  int ret;

  PASSWD_TEST_LOG("Test 6: Delete User");

  ret = passwd_deluser("testuser");
  if (ret != 0)
    {
      PASSWD_TEST_ERR("  passwd_deluser failed: %d", ret);
      return -1;
    }

  PASSWD_TEST_LOG("  User deleted successfully");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

static int test_passwd_verify_deleted_user(void)
{
  int ret;

  PASSWD_TEST_LOG("Test 7: Verify Deleted User");

  ret = passwd_verify("testuser", "testpass123");
  if (ret == 1)
    {
      PASSWD_TEST_ERR("  Expected mismatch, but passed");
      return -1;
    }

  PASSWD_TEST_LOG("  Correctly returned error for deleted user");
  PASSWD_TEST_LOG("  PASS");
  return 0;
}

int main(int argc, FAR char *argv[])
{
  int ret;
  int failed = 0;

  PASSWD_TEST_LOG("========== Passwd Test ==========");

  ret = test_passwd_add_user();
  if (ret != 0) failed++;

  ret = test_passwd_verify_user();
  if (ret != 0) failed++;

  ret = test_passwd_verify_wrong_password();
  if (ret != 0) failed++;

  ret = test_passwd_find_nonexistent();
  if (ret != 0) failed++;

  ret = test_passwd_find_existing();
  if (ret != 0) failed++;

  ret = test_passwd_del_user();
  if (ret != 0) failed++;

  ret = test_passwd_verify_deleted_user();
  if (ret != 0) failed++;

  PASSWD_TEST_LOG("==================================");
  if (failed == 0)
    {
      PASSWD_TEST_LOG("All tests passed!");
    }
  else
    {
      PASSWD_TEST_ERR("%d test(s) failed", failed);
    }

  return failed;
}

#endif /* CONFIG_TESTING_PASSWDTEST */
