#include "timeController.hpp"

TimeController::TimeController() : state_(TimeController::State::CALIBRATING),
lastResetDate_(0), needResetRTC_(true)
{
}

HAL_StatusTypeDef TimeController::resetRTCWithTimePacket(TimePacket *time)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_StatusTypeDef status;
    
    // 設定時間
    sTime.Hours = time->hour;
    sTime.Minutes = time->minute;
    sTime.Seconds = time->second;

    status = HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    if (status != HAL_OK) {
        return status;
    }

    // 設定日期
    sDate.Year    = time->year; // RTC 支援後兩位年份
    sDate.Month   = time->month;
    sDate.Date    = time->day;
    sDate.WeekDay = time->weekday;

    status = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return status;
}

void TimeController::manageCalibrationCycle() {
    // // 假設每 24 小時校準一次
    // if (HAL_GetTick() - lastSyncTick > 172800000) {
    //     g_txBuffer.timeRequest = 1; 
    // }
}

void TimeController::updateSystem(TimePacket* time)
{
    switch (state_)
    {
    case TimeController::State::IDLE:
        if (lastResetDate_ != time->day) {
            needResetRTC_ = true;
            lastResetDate_ = time->day;
            state_ = TimeController::State::CALIBRATING;
        }
        break;
    case TimeController::State::CALIBRATING:
        {
            HAL_StatusTypeDef status = resetRTCWithTimePacket(time);
            if (status == HAL_OK)
            {
                state_ = TimeController::State::IDLE;
            }
            break;
        }
    default:
        break;
    }
}

RTC_TimeTypeDef TimeController::getCurrentTime()
{
    RTC_TimeTypeDef sTime = { 0 };
    RTC_DateTypeDef sDate = { 0 };
    if (state_ != TimeController::State::IDLE)
        return sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sTime;
}


