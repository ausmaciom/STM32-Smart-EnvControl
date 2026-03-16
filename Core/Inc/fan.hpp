#pragma once
#include "main.h"

class Fan
{
private:
	GPIO_TypeDef *port_;
	uint8_t pin_;
	bool isRunning_ = false;
	uint32_t start_ = 0;
	static const uint32_t DURATION_ = 300000; // 5分
public:
	Fan(GPIO_TypeDef *port, uint8_t pin);
	void turnOn();
	void turnOFF();
	bool isTimeout();
};