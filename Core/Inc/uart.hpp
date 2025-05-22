#pragma once

#include "main.h"
#include <string.h>
#include <stdio.h>

#define RX_BUFFER_SIZE 4
#define TIME_BUFFER_SIZE 9

// 前向宣告
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

// 狀態機定義
typedef enum {
	IOT_MODE,	// IOT下達指令
    AUTO_MODE	// STM32控制
} SystemState;

typedef struct{
    uint8_t year;   // 年
    uint8_t month;  // 月
    uint8_t day;    // 日
    uint8_t hour;   // 時
    uint8_t minute; // 分
    uint8_t second; // 秒
    uint8_t weekday; // 星期
} timePacket;

// 全域變數宣告
uint8_t commandReceived;
bool uartTxComplete;
extern uint8_t Message[TIME_BUFFER_SIZE];
extern uint8_t rxBuffer[TIME_BUFFER_SIZE];
extern SystemState state;
extern uint8_t iotCommand;
extern uint8_t timeDemand;
extern timePacket timeinfo;

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
    timeDemand = 1;
}

uint16_t convertFloatToInt(float value)
{
    return (uint16_t)(value * 10.0f);
}

bool isValidMessage(bool timeDemand)
{
    if (timeDemand)
        return Message[0] == 0xAA && Message[TIME_BUFFER_SIZE - 1] == 0xFF;
    else
        return Message[0] == 0xAA && Message[RX_BUFFER_SIZE - 1] == 0xFF;
}

void sendSensorDataBinary(float *temperature, float *humidity, uint8_t *timeDemand)
{
	uint16_t temp_int = convertFloatToInt(*temperature);
	uint16_t humi_int = convertFloatToInt(*humidity);

	// 創建二進制數據包
    static uint8_t dataPacket[8]; // 起始標記 + 溫度 + 濕度
    dataPacket[0] = 0xAA;
    dataPacket[1] = 0x55;
    dataPacket[2] = (uint8_t)(temp_int & 0xFF);
    dataPacket[3] = (uint8_t)((temp_int >> 8) & 0xFF);
    dataPacket[4] = (uint8_t)(humi_int & 0xFF);
    dataPacket[5] = (uint8_t)((humi_int >> 8) & 0xFF);
    dataPacket[6] = commandReceived;
    dataPacket[7] = timeDemand;
    commandReceived = 0x11;
    // 使用DMA發送數據
    if (uartTxComplete) {
        uartTxComplete = false;
        HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart1, dataPacket, 8);
        if (status != HAL_OK) {
            uartTxComplete = true;                                 // 如果發送失敗，重置標誌
        }
    }
}

void updateState()
{
    if (isValidMessage(false)) {
        state          = Message[1] ? AUTO_MODE : IOT_MODE;
        iotCommand     = Message[2];
        Message[0]     = 0;
    }
}

HAL_StatusTypeDef receiveTime()
{
    if (isValidMessage(true)) {
        timeinfo.year  = Message[1]; // 年
        timeinfo.month = Message[2];       // 月
        timeinfo.day   = Message[3];       // 日
        timeinfo.weekday = Message[4];    // 星期
        timeinfo.hour  = Message[5];       // 時
        timeinfo.minute = Message[6];     // 分
        timeinfo.second = Message[7];     // 秒
        Message[0] = 0; // 使用後清除
        timeDemand = 0; // 重置時間需求標誌
        return HAL_OK;
    }
    return HAL_ERROR;
}