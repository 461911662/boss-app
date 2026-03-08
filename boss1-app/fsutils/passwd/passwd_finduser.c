/****************************************************************************
 * apps/fsutils/passwd/passwd_find_user.c
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
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <semaphore.h>
#include <nuttx/config.h>
#include "passwd.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: passwd_find_user
 *
 * Description:
 *   Check if a user exists in the password file
 *
 * Input Parameters:
 *   username - The username to check
 *
 * Returned Value:
 *   true if user exists, false otherwise
 *
 ****************************************************************************/

bool passwd_finduser(FAR const char *username)
{
  struct passwd_s pwd;
  PASSWD_SEM_DECL(sem);
  int ret;

  /* Get exclusive access to the /etc/passwd file */

  ret = passwd_lock(&sem);
  if (ret < 0)
    {
      return false;
    }

  ret = passwd_find(username, &pwd);

  passwd_unlock(sem);

  return (ret == 0);
}
