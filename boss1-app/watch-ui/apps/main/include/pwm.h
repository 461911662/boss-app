/**
 * @file watch-ui/apps/main/include/pwm.h
 * @brief PWM 后台应用Worker - 呼吸灯控制
 */

#pragma once

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include "ability/ability.h"
#include <nuttx/timers/pwm.h>
#include "esp32s3_pwm.h"

/****************************************************************************
 * CLASS PROTOTYPES
 ****************************************************************************/

namespace mooncake {

class PwmWorker : public WorkerAbility {
public:
    PwmWorker();
    ~PwmWorker();

    const char *GetName();

    void onCreate() override;
    void onResume() override;
    void onRunning() override;
    void onPause() override;
    void onDestroy() override;

    void setDutyTarget(uint16_t duty);
    uint16_t getDutyTarget() const;
    void setFadeTime(uint32_t time_ms);
    uint32_t getFadeTime() const;
    void startBreath();
    void stopBreath();
    bool isBreathActive();
    void startPwm();      /* 启动PWM（调用PWMIOC_START） */
    void stopPwm();       /* 停止PWM（调用PWMIOC_STOP） */

private:
    int _fd;
    const char* _name;
    bool _breath_active;
    bool _fade_paused;       /* Mooncake框架设置的暂停状态 */
    uint16_t _duty_target;
    uint32_t _fade_time_ms;

    int _pwmInit();      /* 内部初始化：打开设备、配置参数 */
    void _pwmDeinit();   /* 内部反初始化：关闭设备 */
};

}