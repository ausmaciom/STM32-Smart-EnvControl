#include "timeController.hpp"

TimeController::TimeController()
{}

HAL_StatusTypeDef TimeController::resetRTCWithTimePacket(timePacket *time)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_StatusTypeDef status;
    
    // 設定時間
    sTime.Hours   = time->hour;
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
    resetedDate   = sDate.Date;
    // sDate.WeekDay = time->weekday;

    status = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return status;
}

RTC_TimeTypeDef TimeController::getCurrentTime()
{
    RTC_TimeTypeDef sTime = {0};
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    return sTime;
}
