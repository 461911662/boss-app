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
#include <demos/lv_demos.h>

#include "watch_common.h"
#include "face/face_app.h"
#include "login/login_app.h"

using namespace mooncake;

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef enum {
  Apps_State_UnInstall,
  Apps_State_Install,
  Apps_State_Invalid = 9999,
} Apps_State_E;

typedef struct {
  int app_id;
  char app_name[20];
  Apps_Type_E type;
  Apps_State_E state;
} App_Data_S;

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define APP_NAME_LOGIN "Login"
#define APP_NAME_FACE "Face"

/****************************************************************************
 * STATIC VARIABLES
 ****************************************************************************/
static App_Data_S apps_map[] = {
  [Apps_Type_Login] = { -1,    APP_NAME_LOGIN,    Apps_Type_Login,    Apps_State_UnInstall },
  [Apps_Type_Face]  = { -1,    APP_NAME_FACE,      Apps_Type_Face,    Apps_State_UnInstall },
};
static int apps_map_len = sizeof(apps_map)/sizeof(apps_map[0]);

/****************************************************************************
 * GLOBAL VARIABLES
 ****************************************************************************/
Mooncake mc;

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
int mc_get_app_id_by_type(Apps_Type_E type)
{
  return apps_map[type].app_id;
}

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/
static void apps_setup(void)
{
  for (int i = 0; i < apps_map_len; i++) {
    App_Data_S *app_data = &apps_map[i];

    switch (app_data->type)
    {
      case Apps_Type_Login:
        app_data->app_id = mc.installApp(std::make_unique<Login_App>(app_data->app_name));
        break;
      case Apps_Type_Face:
        app_data->app_id = mc.installApp(std::make_unique<Face_App>(app_data->app_name));
        break;
      default:
        LV_LOG_ERROR("Installed app's Type is incorrect: %s", app_data->app_name);
        break;
    }
    if (app_data->app_id < 0) {
      LV_LOG_ERROR("Install app failed: %s", app_data->app_name);
      continue;
    }
    app_data->state = Apps_State_Install;
  }

  // summary apps
  auto app_info_list = mc.getAllAppInfo();
  for (auto& info : app_info_list) {
    LV_LOG_USER("Installed app name: %s", info.name.c_str());
  }
  LV_LOG_USER("Installed app Total Number: %d", app_info_list.size());

  // go app
  MC_UPDATE();
  int app_id = mc_get_app_id_by_type(Apps_Type_Login);
  if (app_id != -1) {
    MC_OPEN_APP(app_id);
  }
  MC_UPDATE();
}

static void apps_teardown(void)
{
  int iRet = 0;
  for (int i = 0; i < apps_map_len; i++) {
    App_Data_S *app_data = &apps_map[i];
    if (app_data->state != Apps_State_Install) {
      continue;
    }
    iRet = mc.uninstallApp(app_data->app_id);
    if (iRet == false) {
      LV_LOG_ERROR("UnInstall app failed: %s", app_data->app_name);
    }
    app_data->app_id = -1;
    app_data->state = Apps_State_UnInstall;
  }
}

/****************************************************************************
 * main
 ****************************************************************************/

extern "C" int main(int argc, FAR char *argv[])
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

  apps_setup();

  lv_nuttx_run(&result);

  apps_teardown();
  lv_disp_remove(result.disp);
  lv_deinit();

  return 0;
}
