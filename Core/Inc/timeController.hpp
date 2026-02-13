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
    uint8_t* timeDemand_;
    
    HAL_StatusTypeDef resetRTCWithTimePacket(timePacket* time);

public:
    TimeController(uint8_t* timeDemand);
    void updateSystem(timePacket* time);
    RTC_TimeTypeDef getCurrentTime();
};

