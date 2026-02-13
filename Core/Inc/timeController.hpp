#pragma once

#include "main.h"

extern RTC_HandleTypeDef hrtc;

class TimeController
{
private:
    enum class State
    {
        IDLE = 0,
        CALIBRATING = 1
    };
    State state_;
    bool needResetRTC_;
    int32_t lastSyncTick;
    uint8_t lastResetDate_;

public:
    TimeController();
    void manageCalibrationCycle();
    void updateSystem(TimePacket* time);
    RTC_TimeTypeDef getCurrentTime();
    HAL_StatusTypeDef resetRTCWithTimePacket(TimePacket *time);
};

