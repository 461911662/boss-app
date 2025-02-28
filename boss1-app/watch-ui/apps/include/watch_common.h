/**
 * @file watch_common.h
 * @brief
 *   watch common head file.
 */

#pragma once
#include "mooncake.h"


/****************************************************************************
 * DEFINES
 ****************************************************************************/
#define MC_UPDATE() mc.update()
#define MC_OPEN_APP(id) mc.openApp(id)
#define MC_CLOSE_APP(id) mc.closeApp(id)
#define MC_UNINSTALL_APP(id) mc.uninstallApp(id)
#define MC_UNINSTALL_ALLAPP() mc.uninstallAllApps()

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
typedef enum {
  Apps_Type_Login,
  Apps_Type_Face,
  Apps_Type_Invalid = 9999,
} Apps_Type_E;

/****************************************************************************
 * GLOBAL VARIABLES
 ****************************************************************************/
extern Mooncake mc;

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/
extern int mc_get_app_id_by_type(Apps_Type_E type);
