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
	uint8_t currentHour;
	uint8_t humidifyTimes;
	bool needHumidify;
	uint32_t start;
	bool isRunning;
public:
    Humidifier(uint8_t currentHour_);
    bool startHumidify(uint8_t currentHour_);
    void updateHumidify();
};