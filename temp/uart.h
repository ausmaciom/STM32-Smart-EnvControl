#ifndef __UART_H
#define __UART_H

#include "main.h"

#define RX_BUFFER_SIZE 10

// 前向宣告
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

// 本作用域變數
static uint8_t rxBuffer[RX_BUFFER_SIZE];
static uint8_t localRxBuffer[RX_BUFFER_SIZE];
static SensorPacket txBuffer;
static uint8_t g_uartReady;

extern uint8_t g_iotCommand_;
extern uint8_t g_timeRequest;
extern TimePacket g_timeinfo;

enum class CommStatus : uint8_t {
    IDLE    = 0x11,
    RECEIVED = 0x22
};

// 本作用域函數
static uint16_t convertFloatToInt(float value);

// 函數宣告
void sendSensorDataBinary();
HAL_StatusTypeDef isRxMessageOK();

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);


#endif /* __UART_H */