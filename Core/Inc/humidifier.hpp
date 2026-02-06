#include "main.h"

class Humidifier
{
private:
	uint8_t currentHour;
	uint8_t humidifyTimes;
	bool needHumidify;
	uint32_t start;
	bool isRunning;
public:
    Humidifier(uint8_t currentHour_) : currentHour(currentHour_), humidifyTimes(0), needHumidify(false)
										now(0), start(0), isRunning(false)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    }
    bool startHumidify(uint8_t currentHour_)
    {
        if (currentHour != currentHour_) {
            currentHour   = currentHour_;
            humidifyTimes = 0;
        }
		if (humidifyTimes < 4 && !isRunning)
		{
            needHumidify = true;
			humidifyTimes++;
			return true;
        }
		return false;
    }
    void updateHumidify()
	{
        if (!needHumidify) 
		{
			return;
		}

        if (!isRunning) {
            start     = HAL_GetTick();
            isRunning = true;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
        }

        uint32_t now = HAL_GetTick();
		if(now - start >= 5000)
		{
			start = 0;
            isRunning = false;
            needHumidify = false;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
        }
    }
};