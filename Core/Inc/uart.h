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
static bool g_uartReady;

extern uint8_t g_iotCommand;
extern uint8_t g_timeRequest;
extern timePacket g_timeinfo;

#pragma pack(push, 1)
typedef struct{
    uint8_t year;   // 年
    uint8_t month;  // 月
    uint8_t day;    // 日
    uint8_t hour;   // 時
    uint8_t minute; // 分
    uint8_t second; // 秒
    uint8_t weekday; // 星期
} timePacket;

typedef struct 
{
    uint16_t startMarker;   // 起始標記 0xAA55
    uint16_t hotTemp;       // 熱區溫度
    uint16_t hotHumid;      // 熱區濕度
    uint16_t coldTemp;      // 冷區溫度
    uint16_t coldHumid;     // 冷區濕度
    uint8_t ackStatus;      // 確認狀態
    uint8_t timeRequest;    // 時間需求
} SensorPacket;
#pragma pack(pop)

enum class CommStatus : uint8_t {
    IDLE    = 0x11,
    RECEIVED = 0x22
}

// 本作用域函數
static uint16_t convertFloatToInt(float value);

// 函數宣告
void sendSensorDataBinary();
HAL_StatusTypeDef isRxMessageOK();

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);


#endif /* __UART_H */