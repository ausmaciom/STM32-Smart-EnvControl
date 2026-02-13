#pragma once
#include "main.h"
/*
 ******************************************************************************
 * @file           : humidifier.hpp
 * @brief          : Humidifier control class header
 ******************************************************************************
 * @attention
 *
*/
class Humidifier
{
private:
    GPIO_TypeDef *port_; 
	uint16_t pin_;
    uint8_t currentHour_;
    uint8_t humidifyTimes_;
    bool needHumidify_;
	uint32_t start_;
	uint32_t end_;
	bool isRunning_;
    static const uint32_t HUMIDIFY_DURATION = 5000; // 5秒
	static const uint32_t HUMIDIFY_INTERVAL = 900000U; // 15分
    static const uint8_t MAX_TIMES_PER_HOUR = 4;    // 每小時上限
public:
    Humidifier();
	void begin(GPIO_TypeDef *port, uint16_t pin, uint8_t initialHour);
    void updateState(uint8_t currentHour);
    void humidify();
};