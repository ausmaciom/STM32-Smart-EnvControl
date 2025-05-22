#pragma once

#include "main.h"
#include "tim.h"
#include "uart.hpp"

typedef struct {
    RTC_TimeTypeDef time; // 時間
    RTC_DateTypeDef date; // 日期
} timeStruct;

extern TIM_HandleTypeDef htim2;

class FuzzyController
{
private:
    typedef enum
    {
        COLD,
        NORMAL,
        HOT
    } FuzzySet;
    typedef enum
    {
        COOLING = 0, // 冷卻
        WEAK_HEATING   = 1, // 弱加熱
        MED_HEATING      = 2, // 中加熱
        STRONG_HEATING   = 3, // 強加熱
    } FuzzySetOutput;
    FuzzySetOutput NormalruleTable[3][3] = {
        // 冷區: 冷 正常 熱                            熱區
        {STRONG_HEATING, MED_HEATING, WEAK_HEATING}, // 冷
        {MED_HEATING, WEAK_HEATING, COOLING},        // 正常
        {WEAK_HEATING, COOLING, COOLING}             // 熱
    };
    float controlOutput[4]; // 控制輸出
    float maxNightSafetyTemp; // 晚上安全溫度
    float maxDaySafetyTemp;   // 白天安全溫度

    FuzzySet fuzzifyTemperature(float temperature, float targetTemp)
    {
        if (temperature < targetTemp - 2)
            return COLD;
        else if (temperature > targetTemp + 2)
            return HOT;
        else
            return NORMAL;
    }

public:
    FuzzyController(float maxNightTemp, float maxDayTemp)
    {
        maxNightSafetyTemp = maxNightTemp;
        maxDaySafetyTemp   = maxDayTemp;
        controlOutput = {0, 4, 8, 15};
    }
    struct ControlOutput
    {
        float dutyCycle;
        bool safetyTriggered; // 安全保護是否觸發
    };
    ControlOutput fuzzyControl(float coldTemperature, float coldTarget, float hotTemperature, float hotTarget, bool isDay)
    {
        ControlOutput output;
        FuzzySet coldTempSet = fuzzifyTemperature(coldTemperature, coldTarget);
        FuzzySet hotTempSet = fuzzifyTemperature(hotTemperature, hotTarget);

        // 根據模糊規則表獲取控制輸出
        output.dutyCycle       = controlOutput[NormalruleTable[coldTempSet][hotTempSet]];
        output.safetyTriggered = false;

        // 檢查安全保護
        if (isDay)
        {
            if (hotTemperature < maxDaySafetyTemp)
                output.safetyTriggered = true;
        }
        else
        {
            if (hotTemperature < maxNightSafetyTemp)
                output.safetyTriggered = true;
        }
        return output;
    }
}

class TempController
{
private:
    FuzzyController fuzzyController;
    uint8_t currentDay;
    timeStruct* timeptr;
    uint8_t* needResetRTC;
    float hotTarget, coldTarget;
    bool isDay;
    void PWMSetDutyCycle(float duty_cycle)
    {
        // 限制在有效範圍內
        if (duty_cycle < 0) duty_cycle = 0;
        if (duty_cycle > 100) duty_cycle = 100;

        // 計算 CCR 值
        uint32_t pulse = (uint32_t)((duty_cycle / 100.0f) * (__HAL_TIM_GET_AUTORELOAD(&htim2)));

        // 設置新的占空比
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
    }
    bool needResetRTC()
    {
        if (timeptr->date.Date != currentDay)
        {
            currentDay = timeptr->date.Date;
            return true;
        }
        return false;
    }
    void switchTargetTemp()
    {
        if (timeptr->time.Hours >= 7 && timeptr->time.Hours < 17)
        {
            hotTarget = 30.0f; // 白天目標溫度
            coldTarget = 26.5f; // 白天目標溫度
            isDay = true;
        }
        else
        {
            hotTarget = 26.0f; // 晚上目標溫度
            coldTarget = 24.0f; // 晚上目標溫度
            isDay = false;
        }
    }

public:
    TempController(timeStruct *time, uint8_t *timeDemand, float maxNightTemp, float maxDayTemp)
        : fuzzyController(maxNightTemp, maxDayTemp), timeptr(time), needResetRTC(timeDemand)
    {
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
        PWMSetDutyCycle(0); // 初始占空比為 0
        currentDay = time->date.Date;
        switchTargetTemp();
    }
    void setDutyCycle(float coldTemp, float hotTemp)
    {
        switchTargetTemp();
        FuzzyController::ControlOutput output = fuzzyController.fuzzyControl(coldTemp, coldTarget, hotTemp, hotTarget, isDay);
        if (output.safetyTriggered)
        {
            PWMSetDutyCycle(0); // 安全保護，設置占空比為 0
        }
        else
        {
            PWMSetDutyCycle(output.dutyCycle);
        }
    }
}






