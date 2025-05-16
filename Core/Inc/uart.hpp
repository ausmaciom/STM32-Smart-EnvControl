#pragma once

#include "main.h"
#include <string.h>
#include <stdio.h>

#define RX_BUFFER_SIZE 16


// 前向宣告
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

// 狀態機定義
typedef enum {
	IOT_MODE,	// IOT下達指令
    AUTO_MODE	// STM32控制
} SystemState;

// 全域變數宣告
uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t receivedCMD[RX_BUFFER_SIZE];
uint8_t iotCommand;
SystemState state;
uint16_t commandReceived;
bool uartTxComplete;

// 函數宣告
bool confirmState(void);
void updateState(void);
void sendSensorDataBinary(float *temperature, float *humidity);

void initUART(void)
{
    uartTxComplete = true;
    commandReceived = 0x1000;
    state = AUTO_MODE;
    iotCommand = 0;
}

uint16_t convertFloatToInt(float value)
{
    return (uint16_t)(value * 10.0f);
}

bool isValidMessage(void)
{
    return receivedCMD[0] == 0xAA && receivedCMD[RX_BUFFER_SIZE-1] == 0xFF;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) 
	{
    	memcpy(receivedCMD, rxBuffer, RX_BUFFER_SIZE);
		commandReceived = 0x1001;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uartTxComplete = true;
    }
}

void sendSensorDataBinary(float *temperature, float *humidity) 
{
	uint16_t temp_int = convertFloatToInt(*temperature);
	uint16_t humi_int = convertFloatToInt(*humidity);

	// 創建二進制數據包
	static uint16_t dataPacket[4]; // 起始標記 + 溫度 + 濕度
	dataPacket[0] = 0xAA55; // 起始標記
	dataPacket[1] = temp_int;
	dataPacket[2] = humi_int;
	dataPacket[3] = commandReceived;
    // 使用DMA發送數據
    if (uartTxComplete) {
        uartTxComplete = false;
        HAL_UART_Transmit_DMA(&huart1, (uint8_t *)dataPacket, 8); // 3個Half-Word = 6個Byte
    }
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED開
}

bool confirmState(void)
{
    commandReceived = 0x1000;
    uint8_t iotCount = 0, autoCount = 0;
	for (uint8_t i = 0; i < RX_BUFFER_SIZE; i++)
	{
		if (*(receivedCMD + i) == 0)
			iotCount++;
		else if (*(receivedCMD + i) == 1)
			autoCount++;
		else
			iotCommand = *(receivedCMD + i);
	}
	return autoCount > iotCount; // auto : 1 ; iot : 0
}

void updateState(void)
{
    if (isValidMessage())
        state = confirmState() ? AUTO_MODE : IOT_MODE;
}
