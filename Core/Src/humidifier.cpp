#include "humidifier.hpp"
/*
    ******************************************************************************
    * @file           : humidifier.cpp
    * @brief          : Humidifier control class implementation
    ******************************************************************************
    * @attention
    *
*/
Humidifier::Humidifier() : humidifyTimes_(0), needHumidify_(false), start_(0), isRunning_(false), end_(0)
{
}

void Humidifier::begin(GPIO_TypeDef *port, uint16_t pin, uint8_t initialHour)
{
    port_ = port;
    pin_ = pin;
    currentHour_ = initialHour;
    end_         = HAL_GetTick() - HUMIDIFY_INTERVAL;
    HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
}
void Humidifier::updateState(uint8_t currentHour)
{
    if (currentHour_ != currentHour) {
        currentHour_   = currentHour;
        humidifyTimes_ = 0;
    }
    if (humidifyTimes_ < MAX_TIMES_PER_HOUR && !isRunning_) {
        needHumidify_ = true;
    }
}
void Humidifier::humidify()
{
    if (!needHumidify_ && !isRunning_) return;
    uint32_t now = HAL_GetTick();

    if (needHumidify_ && !isRunning_) {
        if (now - end_ >= HUMIDIFY_INTERVAL) { // 冷卻檢查
            start_        = now;
            isRunning_    = true;
            needHumidify_ = false;
            humidifyTimes_++;
            HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_RESET);
            
        } else {
            needHumidify_ = false;
        }
    }

    if (isRunning_) {
        if (now - start_ >= HUMIDIFY_DURATION) {
            isRunning_ = false;
            end_ = now;
            HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
        }
    }
}
