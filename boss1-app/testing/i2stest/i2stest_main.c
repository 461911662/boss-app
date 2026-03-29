/****************************************************************************
 * apps/testing/i2stest/i2stest_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
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

#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <sys/ioctl.h>

#include <nuttx/audio/audio.h>
#include <nuttx/audio/i2s.h>

#define I2STEST_LOG(fmt, ...)    printf("[i2stest] " fmt "\n", ##__VA_ARGS__)
#define I2STEST_ERR(fmt, ...)    printf("[i2stest] ERROR: " fmt "\n", ##__VA_ARGS__)

#define I2S_TEST_DEV_PATH_RX     "/dev/i2schar0"
#define I2S_TEST_DEV_PATH_TX    "/dev/i2schar1"
#define I2S_TEST_BUFSIZE        512
#define I2S_TEST_LOOP_COUNT     5

#define I2S_TEST_SAMPLE_RATE     44100
#define I2S_TEST_CHANNELS        1
#define I2S_TEST_BPS             16

#ifdef CONFIG_TESTING_I2STEST_TX
#  ifndef CONFIG_TESTING_I2STEST_TX_BUFFERS
#    define CONFIG_TESTING_I2STEST_TX_BUFFERS 5
#  endif
#  ifndef CONFIG_TESTING_I2STEST_TX_BUFSIZE
#    define CONFIG_TESTING_I2STEST_TX_BUFSIZE 512
#  endif
#  ifndef CONFIG_TESTING_I2STEST_FREQUENCY
#    define CONFIG_TESTING_I2STEST_FREQUENCY 440
#  endif
#endif

enum i2stest_mode
{
  I2STEST_MODE_RX,
  I2STEST_MODE_TX
};

static void print_usage(void)
{
  printf("Usage: i2stest [OPTIONS]\n");
  printf("\nOptions:\n");
  printf("  -t <rx|tx>  Select mode: rx (receive/microphone) or tx (transmit/speaker). Default: rx\n");
#ifdef CONFIG_TESTING_I2STEST_TX
  printf("  -f <freq>   Sine wave frequency in Hz (TX mode only). Default: %d\n",
         CONFIG_TESTING_I2STEST_FREQUENCY);
  printf("  -e          Export sine wave data to file /tmp/sine_wave.raw (TX mode only)\n");
#endif
  printf("  -h          Show this help message\n");
}

static int do_rx_test(void)
{
  int fd;
  int ret;
  struct ap_buffer_s *apb1;
  struct audio_buf_desc_s buf_desc1;
  int loop;
  int i;

  I2STEST_LOG("I2S RX (Microphone) Test Starting...");
  I2STEST_LOG("Device: %s", I2S_TEST_DEV_PATH_RX);

  fd = open(I2S_TEST_DEV_PATH_RX, O_RDONLY);
  if (fd < 0)
    {
      I2STEST_ERR("Failed to open %s: %d", I2S_TEST_DEV_PATH_RX, errno);
      return EXIT_FAILURE;
    }

  I2STEST_LOG("Device opened successfully, fd=%d", fd);

  buf_desc1.numbytes = I2S_TEST_BUFSIZE;
  buf_desc1.u.pbuffer = &apb1;

  ret = apb_alloc(&buf_desc1);
  if (ret < 0)
    {
      I2STEST_ERR("Failed to allocate buffer: %d", ret);
      close(fd);
      return EXIT_FAILURE;
    }

  I2STEST_LOG("Buffer allocated: apb=%p, nmaxbytes=%u", apb1, apb1->nmaxbytes);
  I2STEST_LOG("Starting to receive %d samples...", I2S_TEST_LOOP_COUNT);

  for (loop = 0; loop < I2S_TEST_LOOP_COUNT; loop++)
    {
      I2STEST_LOG("=== Loop %d ===", loop + 1);

      ret = read(fd, apb1, sizeof(struct ap_buffer_s) + I2S_TEST_BUFSIZE);
      if (ret < 0)
        {
          I2STEST_ERR("read failed: %d", errno);
          continue;
        }

      I2STEST_LOG("read returned: %d bytes, apb->nbytes=%u", ret, apb1->nbytes);

      for (i = 0; i < apb1->nbytes; i++)
        {
          printf("%02x ", apb1->samp[i]);
          if ((i % 16) == 15)
            {
              printf("\n");
            }
        }
      printf("\n");

      I2STEST_LOG("Sleeping for 100ms...");
      usleep(100 * 1000);
    }

  apb_free(apb1);
  close(fd);

  I2STEST_LOG("RX Test completed");
  return EXIT_SUCCESS;
}

#ifdef CONFIG_TESTING_I2STEST_TX
static int generate_sine_wave(int16_t *buffer, int num_samples,
                              int sample_rate, int frequency)
{
  double phase_increment = (2.0 * M_PI * frequency) / sample_rate;
  double phase = 0.0;
  int i;

  for (i = 0; i < num_samples; i++)
    {
      int16_t sample = (int16_t)(32767.0 * sin(phase));
      buffer[i] = sample;
      phase += phase_increment;
      if (phase >= 2.0 * M_PI)
        {
          phase -= 2.0 * M_PI;
        }
    }

  return 0;
}

static int do_tx_test(int frequency)
{
  int fd;
  int ret;
  struct ap_buffer_s *apb;
  struct audio_buf_desc_s buf_desc;
  int num_samples;
  int loop;
  int bufsize;
  int nwritten;

  I2STEST_LOG("I2S TX (Speaker) Test Starting...");
  I2STEST_LOG("Device: %s", I2S_TEST_DEV_PATH_TX);
  I2STEST_LOG("Frequency: %d Hz", frequency);
  I2STEST_LOG("Sample Rate: %d Hz", I2S_TEST_SAMPLE_RATE);
  I2STEST_LOG("Channels: %d", I2S_TEST_CHANNELS);
  I2STEST_LOG("Bits per Sample: %d", I2S_TEST_BPS);

  fd = open(I2S_TEST_DEV_PATH_TX, O_WRONLY);
  if (fd < 0)
    {
      I2STEST_ERR("Failed to open %s: %d", I2S_TEST_DEV_PATH_TX, errno);
      return EXIT_FAILURE;
    }

  I2STEST_LOG("Device opened successfully, fd=%d", fd);

  ret = ioctl(fd, I2SIOC_STXDATAWIDTH, I2S_TEST_BPS);
  if (ret < 0)
    {
      I2STEST_ERR("Failed to set data width %d: %d", I2S_TEST_BPS, errno);
      close(fd);
      return EXIT_FAILURE;
    }

  ret = ioctl(fd, I2SIOC_STXCHANNELS, I2S_TEST_CHANNELS);
  if (ret < 0)
    {
      I2STEST_ERR("Failed to set channels %d: %d", I2S_TEST_CHANNELS, errno);
      close(fd);
      return EXIT_FAILURE;
    }

  ret = ioctl(fd, I2SIOC_STXSAMPLERATE, I2S_TEST_SAMPLE_RATE);
  if (ret < 0)
    {
      I2STEST_ERR("Failed to set sample rate %d: %d", I2S_TEST_SAMPLE_RATE, errno);
      close(fd);
      return EXIT_FAILURE;
    }

  I2STEST_LOG("I2S configured: rate=%d, channels=%d, width=%d",
              I2S_TEST_SAMPLE_RATE, I2S_TEST_CHANNELS, I2S_TEST_BPS);

  num_samples = CONFIG_TESTING_I2STEST_TX_BUFSIZE / 2;
  buf_desc.numbytes = CONFIG_TESTING_I2STEST_TX_BUFSIZE;
  buf_desc.u.pbuffer = &apb;

  ret = apb_alloc(&buf_desc);
  if (ret < 0)
    {
      I2STEST_ERR("Failed to allocate buffer: %d", ret);
      close(fd);
      return EXIT_FAILURE;
    }

  I2STEST_LOG("Buffer allocated: apb=%p, nmaxbytes=%u",
              apb, apb->nmaxbytes);

  ret = generate_sine_wave((int16_t *)apb->samp, num_samples,
                           I2S_TEST_SAMPLE_RATE, frequency);
  if (ret < 0)
    {
      I2STEST_ERR("Failed to generate sine wave: %d", ret);
      apb_free(apb);
      close(fd);
      return EXIT_FAILURE;
    }

  apb->nbytes = CONFIG_TESTING_I2STEST_TX_BUFSIZE;
  bufsize = sizeof(struct ap_buffer_s) + CONFIG_TESTING_I2STEST_TX_BUFSIZE;

  I2STEST_LOG("Starting to send %d buffers...",
              CONFIG_TESTING_I2STEST_TX_BUFFERS);

  for (loop = 0; loop < CONFIG_TESTING_I2STEST_TX_BUFFERS; loop++)
    {
      I2STEST_LOG("=== Loop %d ===", loop + 1);

      do
        {
          fflush(stdout);

          nwritten = write(fd, apb, bufsize);
          if (nwritten < 0)
            {
              int errcode = errno;
              if (errcode != EINTR)
                {
                  I2STEST_ERR("write failed: %d", errcode);
                  apb_free(apb);
                  close(fd);
                  return EXIT_FAILURE;
                }
            }
          else if (nwritten != bufsize)
            {
              I2STEST_ERR("partial write: %d", nwritten);
              apb_free(apb);
              close(fd);
              return EXIT_FAILURE;
            }
          else
            {
              I2STEST_LOG("Sent buffer %d (%d bytes)", loop + 1, nwritten);
            }
        }
      while (nwritten != bufsize);

      usleep(100 * 1000);
    }

  apb_free(apb);
  close(fd);

  I2STEST_LOG("TX Test completed");
  return EXIT_SUCCESS;
}
#endif

int main(int argc, char *argv[])
{
  enum i2stest_mode mode = I2STEST_MODE_RX;
  int opt;
  int ret;

#ifdef CONFIG_TESTING_I2STEST_TX
  int frequency = CONFIG_TESTING_I2STEST_FREQUENCY;
  int export_data = false;
#endif

  while ((opt = getopt(argc, argv, "t:f:he")) != -1)
    {
      switch (opt)
        {
          case 't':
            if (strcmp(optarg, "tx") == 0)
              {
                mode = I2STEST_MODE_TX;
              }
            else if (strcmp(optarg, "rx") == 0)
              {
                mode = I2STEST_MODE_RX;
              }
            else
              {
                I2STEST_ERR("Invalid mode: %s (use 'rx' or 'tx')", optarg);
                print_usage();
                return EXIT_FAILURE;
              }
            break;

          case 'f':
#ifdef CONFIG_TESTING_I2STEST_TX
            frequency = atoi(optarg);
            if (frequency <= 0 || frequency > 20000)
              {
                I2STEST_ERR("Invalid frequency: %d (use 1-20000)", frequency);
                return EXIT_FAILURE;
              }
#else
            I2STEST_ERR("TX mode not enabled. Rebuild with CONFIG_TESTING_I2STEST_TX=y");
            return EXIT_FAILURE;
#endif
            break;

          case 'e':
#ifdef CONFIG_TESTING_I2STEST_TX
            export_data = true;
#else
            I2STEST_ERR("TX mode not enabled. Rebuild with CONFIG_TESTING_I2STEST_TX=y");
            return EXIT_FAILURE;
#endif
            break;

          case 'h':
            print_usage();
            return EXIT_SUCCESS;

          default:
            print_usage();
            return EXIT_FAILURE;
        }
    }

  if (mode == I2STEST_MODE_RX)
    {
      ret = do_rx_test();
    }
#ifdef CONFIG_TESTING_I2STEST_TX
  else
    {
      ret = do_tx_test(frequency);
    }
#else
  else
    {
      I2STEST_ERR("TX mode not enabled. Rebuild with CONFIG_TESTING_I2STEST_TX=y");
      ret = EXIT_FAILURE;
    }
#endif

  return ret;
}
