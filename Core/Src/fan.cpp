#include "fan.hpp"
/*
	******************************************************************************
	* @file           : fan.cpp
	* @brief          : Fan control class implementation
	******************************************************************************
	* @attention
	*
*/
Fan::Fan(GPIO_TypeDef* port, uint8_t pin) :
	port_(port), pin_(pin)
{
}

void Fan::turnOFF()
{
	if (!isRunning_) return;
	isRunning_ = false;
	HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_SET);
}

void Fan::turnOn()
{
	if (isRunning_) return;
	isRunning_ = true;
	start_ = HAL_GetTick();
	HAL_GPIO_WritePin(port_, pin_, GPIO_PIN_RESET);
}

bool Fan::isTimeout()
{
	uint32_t now = HAL_GetTick();
	return (now - start_ >= DURATION_);
}