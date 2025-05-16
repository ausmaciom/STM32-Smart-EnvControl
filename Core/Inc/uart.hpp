#pragma once

#include "main.h"
#include <string.h>
#include <stdio.h>

#define RX_BUFFER_SIZE 4


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
uint8_t commandReceived;
bool uartTxComplete;

// 函數宣告
bool confirmState(void);
void updateState(void);
void sendSensorDataBinary(float *temperature, float *humidity);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

void initUART(void)
{
    uartTxComplete = true;
    commandReceived = 0x11;
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

void sendSensorDataBinary(float *temperature, float *humidity) 
{
	uint16_t temp_int = convertFloatToInt(*temperature);
	uint16_t humi_int = convertFloatToInt(*humidity);

	// 創建二進制數據包
    static uint8_t dataPacket[7]; // 起始標記 + 溫度 + 濕度
    dataPacket[0] = 0x55;
    dataPacket[1] = 0xAA;
    dataPacket[2] = (uint8_t)(temp_int & 0xFF);
    dataPacket[3] = (uint8_t)((temp_int >> 8) & 0xFF);
    dataPacket[4] = (uint8_t)(humi_int & 0xFF);
    dataPacket[5] = (uint8_t)((humi_int >> 8) & 0xFF);
    dataPacket[6] = commandReceived;
    commandReceived = 0x11;
    // 使用DMA發送數據
    if (uartTxComplete) {
        uartTxComplete = false;
        HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart1, dataPacket, 7);
        if (status != HAL_OK) {
            uartTxComplete = true;                                 // 如果發送失敗，重置標誌
        }
    }
}

bool confirmState(void)
{
    uint8_t iotCount = 0, autoCount = 0;
	for (uint8_t i = 0; i < RX_BUFFER_SIZE; i++)
	{
		if (*(receivedCMD + i) == 0)
			iotCount++;
		else if (*(receivedCMD + i) == 1)
			autoCount++;
        else if (*(receivedCMD + i) != 0xAA && *(receivedCMD + i) != 0xFF)
            iotCommand = *(receivedCMD + i);
	}
    receivedCMD[0] = 0;
    return autoCount > iotCount; // auto : 1 ; iot : 0
}

void updateState(void)
{
    if (isValidMessage())
        state = confirmState() ? AUTO_MODE : IOT_MODE;
}
