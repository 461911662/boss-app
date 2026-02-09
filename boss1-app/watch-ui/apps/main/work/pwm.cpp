/**
 * @file watch-ui/apps/main/work/pwm.cpp
 * @brief PWM 后台应用Worker - 呼吸灯控制实现
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include "../include/pwm.h"

#include "watchui/log.h"
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../include/esp32s3_pwm.h"

/****************************************************************************
 * Logging Macros
 ****************************************************************************/
#define PWM_LOG(fmt, ...)    appinfo("[PwmWorker] " fmt, ##__VA_ARGS__)
#define PWM_DBG(fmt, ...)   appdbg("[PwmWorker] " fmt, ##__VA_ARGS__)
#define PWM_WARN(fmt, ...)  appwarn("[PwmWorker] " fmt, ##__VA_ARGS__)
#define PWM_ERR(fmt, ...)   apperr("[PwmWorker] " fmt, ##__VA_ARGS__)

/****************************************************************************
 * Class Methods
 ****************************************************************************/

namespace mooncake {

PwmWorker::PwmWorker()
    : _fd(-1)
    , _name("PwmWorker")
    , _breath_active(false)
    , _fade_paused(false)
    , _duty_target(32768)
    , _fade_time_ms(1000)
{
    PWM_DBG("PwmWorker()");
}

PwmWorker::~PwmWorker()
{
    PWM_DBG("~PwmWorker()");
}

const char *PwmWorker::GetName()
{
    return _name;
}

void PwmWorker::onCreate()
{
    PWM_DBG("onCreate");

    if (_pwmInit() < 0) {
        PWM_ERR("PWM init failed");
        return;
    }

    startPwm();
    startBreath();
}

void PwmWorker::onDestroy()
{
    PWM_DBG("onDestroy");
    _breath_active = false;
    stopBreath();
    stopPwm();
    _pwmDeinit();
}

void PwmWorker::onResume()
{
    PWM_DBG("onResume");
    _fade_paused = false;

    if (_fd < 0 || !_breath_active) {
        return;
    }

    struct pwm_fade_s fade = {
        .channel = 0,
        .auto_reverse = true,
        .target_duty = _duty_target,
        .duty_time = _fade_time_ms
    };

    ioctl(_fd, PWMIOC_START_FADE, &fade);
    PWM_DBG("Breath resumed");
}

void PwmWorker::onRunning()
{
}

void PwmWorker::onPause()
{
    PWM_DBG("onPause");
    _fade_paused = true;

    if (_fd < 0 || !_breath_active) {
        return;
    }

    ioctl(_fd, PWMIOC_STOP_FADE, 0);
    PWM_DBG("Breath paused");
}

void PwmWorker::startBreath()
{
    if (_fd < 0) {
        return;
    }

    struct pwm_info_s info;
    int ret = ioctl(_fd, PWMIOC_GETCHARACTERISTICS, &info);
    if (ret < 0) {
        PWM_ERR("PWMIOC_GETCHARACTERISTICS failed");
        return;
    }

    struct pwm_fade_s fade;
    fade.channel = 0;
    fade.auto_reverse = true;
    fade.target_duty = _duty_target;
    fade.duty_time = _fade_time_ms;

    PWM_LOG("Start fade: current=%u, target=%u, time=%ums",
            (unsigned)info.channels[0].duty, (unsigned)_duty_target, (unsigned)_fade_time_ms);

    ioctl(_fd, PWMIOC_START_FADE, &fade);
    _breath_active = true;
}

void PwmWorker::stopBreath()
{
    if (_fd < 0) {
        return;
    }

    ioctl(_fd, PWMIOC_STOP_FADE, 0);
    _breath_active = false;
}

bool PwmWorker::isBreathActive()
{
    return _breath_active;
}

int PwmWorker::_pwmInit()
{
    struct pwm_info_s info;
    int ret;

    _fd = open("/dev/pwm0", O_RDWR);
    if (_fd < 0) {
        PWM_ERR("Failed to open /dev/pwm0: %d", _fd);
        return -1;
    }

    ret = ioctl(_fd, PWMIOC_GETCHARACTERISTICS, &info);
    if (ret < 0) {
        PWM_ERR("PWMIOC_GETCHARACTERISTICS failed");
        close(_fd);
        _fd = -1;
        return -1;
    }

    PWM_LOG("PWM freq: %lu Hz", (unsigned long)info.frequency);

    info.frequency = 1000;
    info.channels[0].duty = _duty_target;
    info.channels[0].cpol = PWM_CPOL_HIGH;
    info.channels[0].dcpol = PWM_DCPOL_LOW;

    ret = ioctl(_fd, PWMIOC_SETCHARACTERISTICS, &info);
    if (ret < 0) {
        PWM_ERR("PWMIOC_SETCHARACTERISTICS failed");
        close(_fd);
        _fd = -1;
        return -1;
    }

    return 0;
}

void PwmWorker::startPwm()
{
    if (_fd < 0) {
        return;
    }

    int ret = ioctl(_fd, PWMIOC_START, 0);
    if (ret < 0) {
        PWM_ERR("PWMIOC_START failed");
        return;
    }

    PWM_LOG("PWM started");
}

void PwmWorker::stopPwm()
{
    if (_fd < 0) {
        return;
    }

    stopBreath();
    ioctl(_fd, PWMIOC_STOP, 0);
    PWM_LOG("PWM stopped");
}

void PwmWorker::setDutyTarget(uint16_t duty)
{
    if (_breath_active) {
        stopBreath();
    }

    _duty_target = duty;

    if (!_breath_active) {
        startBreath();
    }
}

uint16_t PwmWorker::getDutyTarget() const
{
    return _duty_target;
}

void PwmWorker::setFadeTime(uint32_t time_ms)
{
    if (time_ms < 1) {
        time_ms = 1;
    } else if (time_ms > 1000) {
        time_ms = 1000;
    }

    if (_breath_active) {
        stopBreath();
    }

    _fade_time_ms = time_ms;

    if (!_breath_active) {
        startBreath();
    }
}

uint32_t PwmWorker::getFadeTime() const
{
    return _fade_time_ms;
}

void PwmWorker::_pwmDeinit()
{
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
        PWM_LOG("PWM deinitialized");
    }
}

} // namespace mooncake
