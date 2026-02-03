/****************************************************************************
 * apps/system/nsh/nsh_main.c
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

#include <nuttx/config.h>

#include <spawn.h>
#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/boardctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nshlib/nshlib.h"
#include "watchui/log.h"
#include "watchui_c2cxx.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_main
 *
 * Description:
 *   This is the main logic for the case of the NSH task.  It will perform
 *   one-time NSH initialization and start an interactive session on the
 *   current console device.
 *
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  struct sched_param param;
  int ret = 0;

  /* Check the task priority that we were started with */

  sched_getparam(0, &param);
  if (param.sched_priority != CONFIG_EXAMPLES_WATCHUI_NSH_PRIORITY)
    {
      /* If not then set the priority to the configured priority */

      param.sched_priority = CONFIG_EXAMPLES_WATCHUI_NSH_PRIORITY;
      sched_setparam(0, &param);
    }

  /* Initialize the NSH library */

  nsh_initialize();

  /* go application */
#ifdef CONFIG_WATCHUI

  posix_spawnattr_t attr;
  pid_t pid;

  ret = posix_spawnattr_init(&attr);
  if (ret != 0) {
      apperr("posix_spawnattr_init for nsh.\n");
      return ret;
  }

  param.sched_priority = CONFIG_WATCHUI_PROGNAME_PRIORITY;
  ret = posix_spawnattr_setschedparam(&attr, &param);
  if (ret != 0) {
      apperr("posix_spawnattr_setschedparam for nsh.\n");
      return ret;
  }

  ret = posix_spawnattr_setstacksize(&attr,
                                          CONFIG_WATCHUI_PROGNAME_STACKSIZE);
  if (ret != 0) {
      apperr("posix_spawnattr_setstacksize for nsh.\n");
      return ret;
  }

#if CONFIG_RR_INTERVAL > 0
  ret = posix_spawnattr_setschedpolicy(&attr, SCHED_RR);
  if (ret != 0) {
      apperr("posix_spawnattr_setschedpolicy for nsh.\n");
      return ret;
  }

  ret = posix_spawnattr_setflags(&attr,
                                      POSIX_SPAWN_SETSCHEDPARAM |
                                      POSIX_SPAWN_SETSCHEDULER);
  if (ret != 0) {
      apperr("posix_spawnattr_setflags for nsh.\n");
      return ret;
  }
#else
  ret = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSCHEDPARAM);
  if (ret != 0) {
      apperr("posix_spawnattr_setflags for nsh.\n");
      return ret;
  }
#endif

  appinfo("1%s started, pid=%d\n", CONFIG_WATCHUI_PROGNAME, pid);
  pid = task_spawn(CONFIG_WATCHUI_PROGNAME, watchui_main, NULL, &attr, NULL, environ);
  if (pid < 0) {
      apperr("task_spawn for nsh.\n");
      return ret;
  }
  appinfo("%s started, pid=%d\n", CONFIG_WATCHUI_PROGNAME, pid);

#endif /* CONFIG_WATCHUI */

#ifdef CONFIG_NSH_CONSOLE
  /* If the serial console front end is selected, run it on this thread */

  ret = nsh_consolemain(argc, argv);

  /* nsh_consolemain() should not return.  So if we get here, something
   * is wrong.
   */

  dprintf(STDERR_FILENO, "ERROR: nsh_consolemain() returned: %d\n", ret);
  ret = 1;
#endif

  return ret;
}
