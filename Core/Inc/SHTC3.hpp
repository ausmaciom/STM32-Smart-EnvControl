#pragma once

#define SHTC3_ADDR			0x70
#define SHTC3_CMD_READ_ID 	0xEFC8
#define SHTC3_CMD_WAKEUP	0x3517
#define SHTC3_CMD_SLEEP		0xB098
#define SHTC3_CMD_RESET 	0x805D
#define SHTC3_CMD_MEAS_TEMP 0x58E0
#define SHTC3_CMD_MEAS_MUD  0x7866

class SHTC3 {
private:
	I2C_HandleTypeDef *hi2c1;

	HAL_StatusTypeDef init_status;

	HAL_StatusTypeDef SHTC3SendCommand(uint16_t cmd)
	{
		uint8_t cmd_buffer[2] = {0};
		
		cmd_buffer[0] = (cmd >> 8) & 0xFF;
		cmd_buffer[1] = cmd & 0xFF;

		return HAL_I2C_Master_Transmit(hi2c1, (SHTC3_ADDR << 1), cmd_buffer, 2, 100);
	}

	HAL_StatusTypeDef SHTC3ReadID(uint16_t *id)
	{
		HAL_StatusTypeDef status;
		uint8_t data[3];  // 2位元組ID + 1位元組CRC

		status = SHTC3SendCommand(SHTC3_CMD_READ_ID);
		if (status != HAL_OK) return status;

		status = HAL_I2C_Master_Receive(hi2c1, (SHTC3_ADDR << 1) | 0x01, data, 3, 100);
		if (status != HAL_OK) return status;

		if (!SHTC3CheckCRC(data, 2, data[2])) return HAL_ERROR;

		*id = ((uint16_t)data[0] << 8) | data[1];

		return HAL_OK;
	}


	uint8_t SHTC3CalculateCRC(uint8_t *data, uint8_t len)
	{
	  uint8_t crc = 0xFF;  // 初始值
	  uint8_t polynomial = 0x31;  // CRC多項式 (x^8 + x^5 + x^4 + 1)

	  for (uint8_t i = 0; i < len; i++) {
	    crc ^= data[i];
	    for (uint8_t j = 0; j < 8; j++) {
	      if (crc & 0x80) {
	        crc = (crc << 1) ^ polynomial;
	      } else {
	        crc = (crc << 1);
	      }
	    }
	  }

	  return crc;
	}

	bool SHTC3CheckCRC(uint8_t *data, uint8_t len, uint8_t checksum)
	{
	  return (SHTC3CalculateCRC(data, len) == checksum);
	}

	HAL_StatusTypeDef SHTC3Init(void)
	{
		HAL_StatusTypeDef status;
		uint16_t id = 0;
		uint8_t retries = 3;
		
	    while(retries--) {
			status = SHTC3SendCommand(SHTC3_CMD_WAKEUP);
	        if (status == HAL_OK) break;
			HAL_Delay(10);
	    }
		if (status != HAL_OK) return status;
		HAL_Delay(1);

		status = SHTC3ReadID(&id);
		if (status != HAL_OK) return status;
		if (id != 0x0807 && id != 0x0887)
			return HAL_ERROR;

		status = SHTC3SendCommand(SHTC3_CMD_RESET);
		if (status != HAL_OK) return status;
		HAL_Delay(10);
		
		return HAL_OK;
	}

public:
	SHTC3(I2C_HandleTypeDef *hi2c)
	{
		this->hi2c1 = hi2c;
		init_status = SHTC3Init();
	}

    HAL_StatusTypeDef getInitStatus() {
        return init_status;
    }

	HAL_StatusTypeDef SHTC3ReadTempHumidity(float *temperature, float *humidity)
	{
		HAL_StatusTypeDef status;
		uint8_t data[6];
		uint16_t raw_humid, raw_temp;

		status = SHTC3SendCommand(SHTC3_CMD_WAKEUP);
		if (status != HAL_OK) return status;
		HAL_Delay(1);

		status = SHTC3SendCommand(SHTC3_CMD_MEAS_TEMP);
		if (status != HAL_OK) return status;
		HAL_Delay(15);

		// 讀取數據（6位元組：濕度+CRC, 溫度+CRC）
		status = HAL_I2C_Master_Receive(hi2c1, (SHTC3_ADDR << 1) | 0x01, data, 6, 100);
		if (status != HAL_OK) return status;

		if(!SHTC3CheckCRC(&data[0], 2, data[2]) || !SHTC3CheckCRC(&data[3], 2, data[5])) {
		    return HAL_ERROR;
		}

		raw_humid = ((uint16_t)data[0] << 8) | data[1];
		raw_temp = ((uint16_t)data[3] << 8) | data[4];

		*humidity = 100.0f * ((float)raw_humid / 65535.0f);  // 轉換為相對濕度百分比
		*temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);  // 轉換為攝氏度

		SHTC3SendCommand(SHTC3_CMD_SLEEP);

		return HAL_OK;
	}
};

