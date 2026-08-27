/****************************************************************************
 * apps/testing/audiotest/audiotest_main.c
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
#include <mqueue.h>
#include <sys/ioctl.h>
#include <time.h>

#include <nuttx/audio/audio.h>
#include <debug.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define AUDIOTEST_LOG(fmt, ...)    audinfo("[audiotest] " fmt, ##__VA_ARGS__)
#define AUDIOTEST_ERR(fmt, ...)    auderr("[audiotest] " fmt, ##__VA_ARGS__)

#define AUDIOTEST_DEV_PATH      "/dev/audio/pcm_in0"
#define AUDIOTEST_MQ_NAME       "/tmp/audiotest"
#define AUDIOTEST_DUMP_BYTES_PER_LINE 16
#define AUDIOTEST_MAX_DUMP_LINES 10

#ifndef CONFIG_TESTING_AUDIOTEST_SAMPLERATE
#  define CONFIG_TESTING_AUDIOTEST_SAMPLERATE 16000
#endif

#ifndef CONFIG_TESTING_AUDIOTEST_CHANNELS
#  define CONFIG_TESTING_AUDIOTEST_CHANNELS 1
#endif

#ifndef CONFIG_TESTING_AUDIOTEST_BPS
#  define CONFIG_TESTING_AUDIOTEST_BPS 16
#endif

#define AUDIOTEST_SAMPLERATE   CONFIG_TESTING_AUDIOTEST_SAMPLERATE
#define AUDIOTEST_CHANNELS      CONFIG_TESTING_AUDIOTEST_CHANNELS
#define AUDIOTEST_BPS           CONFIG_TESTING_AUDIOTEST_BPS

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct audiotest_s
{
  int audio_fd;
  mqd_t mq;
  bool running;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct audiotest_s g_audiotest;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void audiotest_print_info(void)
{
  AUDIOTEST_LOG("Audio Test Configuration:");
  AUDIOTEST_LOG("  Device: %s", AUDIOTEST_DEV_PATH);
  AUDIOTEST_LOG("  Sample Rate: %d Hz", AUDIOTEST_SAMPLERATE);
  AUDIOTEST_LOG("  Channels: %d", AUDIOTEST_CHANNELS);
  AUDIOTEST_LOG("  Bits per Sample: %d", AUDIOTEST_BPS);
}

static int audiotest_open_device(void)
{
  g_audiotest.audio_fd = open(AUDIOTEST_DEV_PATH, O_RDONLY);
  if (g_audiotest.audio_fd < 0)
    {
      AUDIOTEST_ERR("Failed to open %s: %d", AUDIOTEST_DEV_PATH, errno);
      return -1;
    }

  AUDIOTEST_LOG("Audio device opened successfully, fd=%d", g_audiotest.audio_fd);
  return 0;
}

static int audiotest_configure(void)
{
  struct audio_caps_desc_s cap_desc;
  int ret;

  memset(&cap_desc, 0, sizeof(cap_desc));

  cap_desc.caps.ac_len      = sizeof(struct audio_caps_s);
  cap_desc.caps.ac_type     = AUDIO_TYPE_INPUT;
  cap_desc.caps.ac_channels = AUDIOTEST_CHANNELS;

  cap_desc.caps.ac_controls.hw[0] = AUDIOTEST_SAMPLERATE;
  cap_desc.caps.ac_controls.b[3] = (AUDIOTEST_SAMPLERATE >> 16) & 0xff;
  cap_desc.caps.ac_controls.b[2] = AUDIOTEST_BPS;

  ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_CONFIGURE,
               (unsigned long)&cap_desc);
  if (ret < 0)
    {
      AUDIOTEST_ERR("AUDIOIOC_CONFIGURE failed: %d", errno);
      return -1;
    }

  AUDIOTEST_LOG("Audio configured successfully");
  return 0;
}

static int audiotest_setup_mqueue(void)
{
  struct mq_attr attr;
  int ret;

  memset(&attr, 0, sizeof(attr));
  attr.mq_maxmsg  = 16;
  attr.mq_msgsize = sizeof(struct audio_msg_s);

  g_audiotest.mq = mq_open(AUDIOTEST_MQ_NAME, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &attr);
  if (g_audiotest.mq < 0)
    {
      AUDIOTEST_ERR("mq_open failed: %d", errno);
      return -1;
    }

  ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_REGISTERMQ, (unsigned long)g_audiotest.mq);
  if (ret < 0)
    {
      AUDIOTEST_ERR("AUDIOIOC_REGISTERMQ failed: %d", errno);
      mq_close(g_audiotest.mq);
      mq_unlink(AUDIOTEST_MQ_NAME);
      return -1;
    }

  AUDIOTEST_LOG("Message queue registered");
  return 0;
}

static void audiotest_dump_buffer(FAR struct ap_buffer_s *apb)
{
  uint32_t i;
  enum
  {
    DUMP_BYTES = 48
  };

  printf("Dump of buffer:");
  for (i = 0; i < apb->nbytes && i < DUMP_BYTES; i++)
    {
      if (i % AUDIOTEST_DUMP_BYTES_PER_LINE == 0)
        {
          printf("\n%04" PRIx32 ":", i);
        }

      printf("%02x ", apb->samp[i]);
    }

  printf("\n");
  if (i < apb->nbytes)
    {
      printf("... (%u bytes total received)\n", apb->nbytes);
    }
}

static int audiotest_enqueue_buffers(void)
{
  struct ap_buffer_info_s buf_info;
  struct audio_buf_desc_s buf_desc;
  FAR struct ap_buffer_s *apb;
  uint32_t total_bytes;
  struct audio_msg_s msg;
  int nbytes;
  int ret;
  int i;

  ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_GETBUFFERINFO, (unsigned long)&buf_info);
  if (ret < 0)
    {
      buf_info.buffer_size = 512;
      buf_info.nbuffers = 2;
      AUDIOTEST_LOG("GETBUFFERINFO failed, using default: size=%u, count=%u",
                    buf_info.buffer_size, buf_info.nbuffers);
    }
  else
    {
      AUDIOTEST_LOG("Buffer info: size=%u, count=%u", buf_info.buffer_size, buf_info.nbuffers);
    }

  buf_desc.numbytes = buf_info.buffer_size;
  buf_desc.u.pbuffer = &apb;

  AUDIOTEST_LOG("buf_desc.numbytes=%d", buf_desc.numbytes);
  ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_ALLOCBUFFER, (unsigned long)&buf_desc);
  if (ret < 0)
    {
      AUDIOTEST_ERR("AUDIOIOC_ALLOCBUFFER failed: %d", errno);
      return -1;
    }

  buf_desc.u.buffer = apb;
  AUDIOTEST_LOG("Allocated buffer %p", (void *)buf_desc.u.buffer);

  for (i = 0; i < 5; i++)
    {
      ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_ENQUEUEBUFFER, (unsigned long)&buf_desc);
      if (ret < 0)
        {
          AUDIOTEST_ERR("AUDIOIOC_ENQUEUEBUFFER failed: %d", errno);
          return -1;
        }

      total_bytes = 0;
      while (1)
        {
          nbytes = mq_timedreceive(g_audiotest.mq, (char *)&msg, sizeof(msg), NULL, 0);

          if (nbytes > 0)
            {
              if (msg.msg_id == AUDIO_MSG_DEQUEUE)
                {
                  struct ap_buffer_s *apb1 = msg.u.ptr;

                  if (apb1 && apb1->nbytes > 0)
                    {
                      audiotest_dump_buffer(apb1);
                      total_bytes += apb1->nbytes;
                    }
                }
              else if (msg.msg_id == AUDIO_MSG_COMPLETE)
                {
                  AUDIOTEST_LOG("Recording complete message received");
                  break;
                }
            }
          else if (nbytes < 0)
            {
              if (errno != EAGAIN && errno != ETIMEDOUT)
                {
                  AUDIOTEST_ERR("mq_receive error: %d", errno);
                  break;
                }
            }

          if (total_bytes >= buf_desc.numbytes)
            {
              AUDIOTEST_LOG("Buffer %d filled", i);
              break;
            }
        }

      AUDIOTEST_LOG("Enqueued buffer %d", i);
    }

  ioctl(g_audiotest.audio_fd, AUDIOIOC_FREEBUFFER, (uintptr_t)&buf_desc);

  AUDIOTEST_LOG("All buffers enqueued successfully");
  return 0;
}

static int audiotest_start(void)
{
  int ret;

  ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_START, 0);
  if (ret < 0)
    {
      AUDIOTEST_ERR("AUDIOIOC_START failed: %d", errno);
      return -1;
    }

  g_audiotest.running = true;
  AUDIOTEST_LOG("Audio recording started");
  return 0;
}

static int audiotest_stop(void)
{
  int ret;

  g_audiotest.running = false;

  ret = ioctl(g_audiotest.audio_fd, AUDIOIOC_STOP, 0);
  if (ret < 0)
    {
      AUDIOTEST_ERR("AUDIOIOC_STOP failed: %d", errno);
      return -1;
    }

  AUDIOTEST_LOG("Audio recording stopped");
  return 0;
}

static void audiotest_cleanup(void)
{
  if (g_audiotest.mq >= 0)
    {
      mq_close(g_audiotest.mq);
      mq_unlink(AUDIOTEST_MQ_NAME);
      g_audiotest.mq = -1;
    }

  if (g_audiotest.audio_fd >= 0)
    {
      close(g_audiotest.audio_fd);
      g_audiotest.audio_fd = -1;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int ret;

  AUDIOTEST_LOG("===========================================");
  AUDIOTEST_LOG("   Audio Test - PDM Microphone Recording");
  AUDIOTEST_LOG("===========================================");

  /* 初始化全局状态 */
  memset(&g_audiotest, 0, sizeof(g_audiotest));
  g_audiotest.audio_fd = -1;
  g_audiotest.mq = -1;

  /* 打印配置信息 */
  audiotest_print_info();

  /* 打开音频设备 */
  ret = audiotest_open_device();
  if (ret < 0)
    {
      goto err_out;
    }

  /* 配置音频参数 */
  ret = audiotest_configure();
  if (ret < 0)
    {
      goto err_out;
    }

  /* 创建消息队列 */
  ret = audiotest_setup_mqueue();
  if (ret < 0)
    {
      goto err_out;
    }

  /* 开始录音 */
  ret = audiotest_start();
  if (ret < 0)
    {
      goto err_out;
    }

  /* 入队缓冲区 */
  ret = audiotest_enqueue_buffers();
  if (ret < 0)
    {
      goto err_out;
    }

  /* 停止录音 */
  ret = audiotest_stop();
  if (ret < 0)
    {
      goto err_out;
    }

  AUDIOTEST_LOG("===========================================");
  AUDIOTEST_LOG("   Recording Completed Successfully!");
  AUDIOTEST_LOG("===========================================");

  audiotest_cleanup();
  return EXIT_SUCCESS;

err_out:
  AUDIOTEST_ERR("Audio test failed!");

  audiotest_cleanup();
  return EXIT_FAILURE;
}