#include "humidifier.hpp"
/*
    ******************************************************************************
    * @file           : humidifier.cpp
    * @brief          : Humidifier control class implementation
    ******************************************************************************
    * @attention
    *
*/
Humidifier::Humidifier() : start_(0), isRunning_(false), state_(true)
{
}

void Humidifier::begin(GPIO_TypeDef *port, uint16_t pin, uint8_t initialHour)
{
    port_ = port;
    pin_ = pin;
    currentHour_ = initialHour;
    end_         = HAL_GetTick() - INTERVAL_;
    HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
}

void Humidifier::turnON()
{
    if (state_) return;
    state_ = true;
}

void Humidifier::turnOFF()
{
    if (!state_) return;
    state_ = false;
}

bool Humidifier::isCooldown()
{
    uint32_t now = HAL_GetTick();
    if (now - end_ >= INTERVAL_)
        return false;
    else
        return true;
}

bool Humidifier::humidify()
{
    uint32_t now = HAL_GetTick();
    if (!state_) {
        if (isRunning_) {
            isRunning_ = false;
            end_       = now;
            HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET); // 強制關閉
        }
        return isRunning_;
    }
    if (!isRunning_) {
        if (now - end_ >= INTERVAL_) { // 冷卻檢查
            start_        = now;
            isRunning_    = true;
            HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_RESET);
        }
    }
    else {
        if (now - start_ >= DURATION_) {
            isRunning_ = false;
            end_ = now;
            HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
        }
    }
    return isRunning_;
}
