/**
 * @file face_app.h
 * @brief
 *   watch face main head file.
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#pragma once
#include "ability/ability.h"

/****************************************************************************
 * CLASS PROTOTYPES
 ****************************************************************************/
namespace mooncake {

class Face_App : public AppAbility {
public:
    Face_App(char *name);

    void onOpen();
    void onRunning();
    void onClose();
};

}
