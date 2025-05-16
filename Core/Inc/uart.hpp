#pragma once

#include "main.h"
#include <string.h>
#include <stdio.h>

#define RX_BUFFER_SIZE 8


// 前向宣告
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

// 狀態機定義
typedef enum {
	IOT_MODE,         // IOT下達指令
    INTELLIGENT_MODE  // STM32控制
} SystemState;

// 全域變數宣告
uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t receivedCMD[RX_BUFFER_SIZE];
SystemState state{INTELLIGENT_MODE};

uint16_t convertFloatToInt(float value)
{
    return (uint16_t)(value * 10.0f);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        ProcessData(rxBuffer, receivedCMD);
        HAL_UART_Receive_DMA(&huart1, rxBuffer, sizeof(rxBuffer));
    }
}

void processData(uint8_t *rxBuffer, uint8_t *receivedCMD)
{
    memcpy(receivedCMD, rxBuffer, RX_BUFFER_SIZE);
}

void initUART(void)
{
    HAL_UART_Receive_DMA(&huart1, rxBuffer, RX_BUFFER_SIZE);
}

void SendSensorDataBinary(float *temperature, float *humidity) 
{
	uint16_t temp_int = ConvertFloatToInt(*temperature);
	uint16_t humi_int = ConvertFloatToInt(*humidity);

	// 創建二進制數據包
	static uint16_t dataPacket[3]; // 起始標記 + 溫度 + 濕度
	dataPacket[0] = 0xAA55; // 起始標記
	dataPacket[1] = temp_int;
	dataPacket[2] = humi_int;

	// 使用DMA發送數據
	HAL_UART_Transmit_DMA(&huart1, (uint8_t*)dataPacket, 6); // 3個Half-Word = 6個Byte
}

