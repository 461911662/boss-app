/****************************************************************************
 * apps/testing/ledtest/ledtest_main.c
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

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <nuttx/leds/userled.h>

#define LEDTEST_LOG(fmt, ...)    printf("[ledtest] " fmt "\n", ##__VA_ARGS__)
#define LEDTEST_ERR(fmt, ...)    printf("[ledtest] ERROR: " fmt "\n", ##__VA_ARGS__)

#define BLINK_COUNT  5

static void show_usage(const char *progname)
{
  printf("Usage: %s <command> [led_num]\n", progname);
  printf("\nCommands:\n");
  printf("  on <led_num>     Turn on LED\n");
  printf("  off <led_num>   Turn off LED\n");
  printf("  toggle <led_num> Toggle LED\n");
  printf("  blink <led_num>  Blink LED %d times\n", BLINK_COUNT);
  printf("  status           Show LED status\n");
  printf("\nExamples:\n");
  printf("  ledtest on 0\n");
  printf("  ledtest off 0\n");
  printf("  ledtest blink 0\n");
}

static int ledtest_on(int fd, uint32_t led_num)
{
  int32_t ret;
  struct userled_s led;

  led.ul_led = led_num;
  led.ul_on = true;

  ret = ioctl(fd, ULEDIOC_SETLED, (unsigned long)&led);
  if (ret < 0)
    {
      LEDTEST_ERR("Failed to turn on LED%" PRIu32 ": %d", led_num, errno);
      return -1;
    }

  LEDTEST_LOG("LED%" PRIu32 " turned on", led_num);
  return 0;
}

static int ledtest_off(int fd, uint32_t led_num)
{
  int32_t ret;
  struct userled_s led;

  led.ul_led = led_num;
  led.ul_on = false;

  ret = ioctl(fd, ULEDIOC_SETLED, (unsigned long)&led);
  if (ret < 0)
    {
      LEDTEST_ERR("Failed to turn off LED%" PRIu32 ": %d", led_num, errno);
      return -1;
    }

  LEDTEST_LOG("LED%" PRIu32 " turned off", led_num);
  return 0;
}

static int ledtest_toggle(int fd, uint32_t led_num)
{
  int32_t ret;
  userled_set_t ledset;

  ret = ioctl(fd, ULEDIOC_GETALL, (unsigned long)&ledset);
  if (ret < 0)
    {
      LEDTEST_ERR("Failed to get LED status: %d", errno);
      return -1;
    }

  bool is_on = (ledset & (1 << led_num)) != 0;

  if (is_on)
    {
      return ledtest_off(fd, led_num);
    }
  else
    {
      return ledtest_on(fd, led_num);
    }
}

static int ledtest_blink(int fd, uint32_t led_num)
{
  struct userled_s led_on;
  struct userled_s led_off;
  int32_t ret;
  uint32_t i;

  led_on.ul_led = led_num;
  led_on.ul_on = true;

  led_off.ul_led = led_num;
  led_off.ul_on = false;

  LEDTEST_LOG("Blinking LED%" PRIu32 " %d times...", led_num, BLINK_COUNT);

  for (i = 0; i < BLINK_COUNT; i++)
    {
      ret = ioctl(fd, ULEDIOC_SETLED, (unsigned long)&led_on);
      if (ret < 0)
        {
      LEDTEST_ERR("Failed to turn on LED%" PRIu32 ": %d", led_num, errno);
          return -1;
        }

      usleep(500 * 1000);

      ret = ioctl(fd, ULEDIOC_SETLED, (unsigned long)&led_off);
      if (ret < 0)
        {
      LEDTEST_ERR("Failed to turn off LED%" PRIu32 ": %d", led_num, errno);
          return -1;
        }

      usleep(500 * 1000);
    }

  LEDTEST_LOG("Blink complete");
  return 0;
}

static int ledtest_status(int fd)
{
  int32_t ret;
  userled_set_t supported;
  userled_set_t ledset;

  ret = ioctl(fd, ULEDIOC_SUPPORTED, (unsigned long)&supported);
  if (ret < 0)
    {
      LEDTEST_ERR("Failed to get supported LEDs: %d", errno);
      return -1;
    }

  ret = ioctl(fd, ULEDIOC_GETALL, (unsigned long)&ledset);
  if (ret < 0)
    {
      LEDTEST_ERR("Failed to get LED status: %d", errno);
      return -1;
    }

  LEDTEST_LOG("Supported LEDs: 0x%02" PRIx32, supported);
  LEDTEST_LOG("Current LED set: 0x%02" PRIx32, ledset);

  return 0;
}

#ifdef CONFIG_TESTING_LEDTEST
int main(int argc, char *argv[])
{
  int fd;
  int ret;

  if (argc < 2)
    {
      show_usage(argv[0]);
      return EXIT_FAILURE;
    }

  fd = open(CONFIG_TESTING_LEDTEST_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      LEDTEST_ERR("Failed to open %s: %d",
                  CONFIG_TESTING_LEDTEST_DEVPATH, errno);
      return EXIT_FAILURE;
    }

  if (strcmp(argv[1], "on") == 0)
    {
      if (argc < 3)
        {
          LEDTEST_ERR("Missing LED number");
          close(fd);
          return EXIT_FAILURE;
        }

      uint32_t led_num = atoi(argv[2]);
      ret = ledtest_on(fd, led_num);
    }
  else if (strcmp(argv[1], "off") == 0)
    {
      if (argc < 3)
        {
          LEDTEST_ERR("Missing LED number");
          close(fd);
          return EXIT_FAILURE;
        }

      uint32_t led_num = atoi(argv[2]);
      ret = ledtest_off(fd, led_num);
    }
  else if (strcmp(argv[1], "toggle") == 0)
    {
      if (argc < 3)
        {
          LEDTEST_ERR("Missing LED number");
          close(fd);
          return EXIT_FAILURE;
        }

      uint32_t led_num = atoi(argv[2]);
      ret = ledtest_toggle(fd, led_num);
    }
  else if (strcmp(argv[1], "blink") == 0)
    {
      if (argc < 3)
        {
          LEDTEST_ERR("Missing LED number");
          close(fd);
          return EXIT_FAILURE;
        }

      uint32_t led_num = atoi(argv[2]);
      ret = ledtest_blink(fd, led_num);
    }
  else if (strcmp(argv[1], "status") == 0)
    {
      ret = ledtest_status(fd);
    }
  else if (strcmp(argv[1], "help") == 0)
    {
      show_usage(argv[0]);
      ret = 0;
    }
  else
    {
      LEDTEST_ERR("Unknown command: %s", argv[1]);
      show_usage(argv[0]);
      ret = EXIT_FAILURE;
    }

  close(fd);
  return (ret < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
