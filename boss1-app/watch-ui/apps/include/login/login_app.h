/**
 * @file login_app.h
 * @attention
 *   watch login main head file.
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

class Login_App : public AppAbility {
public:
    Login_App(char *name);

    void onOpen();
    void onRunning();
    void onClose();
};

}
