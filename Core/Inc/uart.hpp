#pragma once

#include "main.h"
#include <string.h>
#include <stdio.h>

// 前向宣告
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

// 狀態機定義
typedef enum {
  STATE_IDLE,         // 閒置狀態
  STATE_READ_SENSOR,  // 讀取感測器
  STATE_PROCESS_DATA, // 處理數據
  STATE_SLEEP         // 低功耗模式
} SystemState;


class ESP32
{
private:

	uint16_t ConvertFloatToInt(float value) {
	  // 取小數點第一位並轉為3位數整數
	  // 例如: 25.7℃ -> 257, 42.3% -> 423
	  return (uint16_t)(value * 10.0f);
	}

public:
	ESP32()
	{

	}
	/* 處理來自ESP32的命令 */
	void ProcessCommand(uint16_t cmd) {

	}

	void SendSensorDataBinary(float *temperature, float *humidity) {
	  uint16_t temp_int = ConvertFloatToInt(*temperature);
	  uint16_t humi_int = ConvertFloatToInt(*humidity);

	  // 創建二進制數據包
	  uint16_t dataPacket[3]; // 起始標記 + 溫度 + 濕度
	  dataPacket[0] = 0xAA55; // 起始標記
	  dataPacket[1] = temp_int;
	  dataPacket[2] = humi_int;

	  // 使用DMA發送數據
	  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)dataPacket, 6); // 3個Half-Word = 6個Byte
	}
};
