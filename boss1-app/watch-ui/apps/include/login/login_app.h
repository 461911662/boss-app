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
class Login_App : public AppAbility {

public:
    Login_App(char *name){};

    void onOpen() override;
    void onRunning() override;
    void onClose() override;
};
