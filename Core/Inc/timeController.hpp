#pragma once

#include "main.h"

extern RTC_HandleTypeDef hrtc;

class TimeController
{
    private:
        bool needResetRTC;
        uint8_t resetedDate;
        RTC_TimeTypeDef time; // 時間
        RTC_DateTypeDef date; // 日期
        HAL_StatusTypeDef resetRTCWithTimePacket(timePacket *time);

    public:
        TimeController();
        uint8_t getCurrentHour();
};

