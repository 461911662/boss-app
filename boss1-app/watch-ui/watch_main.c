/****************************************************************************
 * apps/examples/hello/hello_main.c
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
#include <stdio.h>
#include <unistd.h>

#include <lvgl.h>
//#include <demos/lv_demos.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;

  lv_init();

  lv_nuttx_dsc_init(&info);

#ifdef CONFIG_LV_USE_NUTTX_LCD
  info.fb_path = "/dev/lcd0";
#endif
  lv_nuttx_init(&info, &result);

  LV_LOG_USER("Hello,World");

  if (result.disp == NULL)
  {
    LV_LOG_ERROR("lv_demos initialization failure!");
    return 1;
  }

  // lv_obj_t * scr = lv_disp_get_scr_act(NULL);
  // lv_obj_t * label1 =  lv_label_create(scr);
  // lv_label_set_text(label1, "Hello\nworld");
  // lv_obj_center(label1);
  // lv_demo_widgets();
  // lv_demo_music();
  lv_demo_keypad_encoder();
  //lv_demo_stress();

  lv_nuttx_run(&result);
  lv_disp_remove(result.disp);
  lv_deinit();

  return 0;
}
