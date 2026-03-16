#pragma once
#include "main.h"
/*
 ******************************************************************************
 * @file           : SHTC3.hpp
 * @brief          : SHTC3 temperature and humidity sensor driver header
 ******************************************************************************
 * @attention
 *
*/

class SHTC3 {
private:
	I2C_HandleTypeDef *hi2c1_;
	HAL_StatusTypeDef initStatus_;

    HAL_StatusTypeDef sendCommand(uint16_t cmd);
	HAL_StatusTypeDef readID(uint16_t *id);
	uint8_t calculateCRC(uint8_t *data, uint8_t len);
	bool checkCRC(uint8_t *data, uint8_t len, uint8_t checksum);
    uint16_t humidity;
    int16_t temperature;
public:
    static constexpr uint8_t SHTC3_ADDR           = 0x70;
    static constexpr uint16_t SHTC3_CMD_READ_ID   = 0xEFC8;
    static constexpr uint16_t SHTC3_CMD_WAKEUP    = 0x3517;
    static constexpr uint16_t SHTC3_CMD_SLEEP     = 0xB098;
    static constexpr uint16_t SHTC3_CMD_RESET     = 0x805D;
    static constexpr uint16_t SHTC3_CMD_MEAS_TEMP = 0x58E0;
    static constexpr uint16_t SHTC3_CMD_MEAS_MUD  = 0x7866;
    SHTC3(I2C_HandleTypeDef *hi2c);
    HAL_StatusTypeDef begin();
	HAL_StatusTypeDef readTempHumidity();
    int16_t getTemperature() const;
    uint16_t getHumidity() const;
};

