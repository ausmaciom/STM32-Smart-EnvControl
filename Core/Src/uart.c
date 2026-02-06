#include "uart.h"

void initUART(void)
{
    g_commStatus = CommStatus::IDLE;
    g_txBuffer.startMarker = 0x55AA;
    g_txBuffer.ackStatus   = (uint8_t)CommStatus::IDLE;
    g_uartReady            = true;
}

uint16_t convertFloatToInt(float value)
{
    return (uint16_t)(value * 10.0f);
}

bool isValidMessage(bool timeDemand)
{
    if (timeDemand)
        return g_rxBuffer[0] == 0xAA && g_rxBuffer[TIME_BUFFER_SIZE - 1] == 0xFF;
    else
        return g_rxBuffer[0] == 0xAA && g_rxBuffer[RX_BUFFER_SIZE - 1] == 0xFF;
}

void sendSensorDataBinary()
{
    if (!g_uartReady) return;

    g_txBuffer.hotTemp = convertFloatToInt(g_hotTemp);
    g_txBuffer.hotHumid = convertFloatToInt(g_hotHumid);
    g_txBuffer.coldTemp = convertFloatToInt(g_coldTemp);
    g_txBuffer.coldHumid = convertFloatToInt(g_coldHumid);
    g_txBuffer.ackStatus = (uint8_t)g_commStatus;
    g_txBuffer.timeRequest = g_timeDemand;

    if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)&g_txBuffer, sizeof(g_txBuffer)) == HAL_OK) {
        g_uartReady = false; // 傳輸開始，設置標誌為 false
    } else {
        g_uartReady = true; // 傳輸失敗，保持標誌為 true
    }
}

HAL_StatusTypeDef receiveTime()
{
    if (isValidMessage(true)) {
        g_timeinfo.year  = g_rxBuffer[1]; // 年
        g_timeinfo.month = g_rxBuffer[2]; // 月
        g_timeinfo.day   = g_rxBuffer[3]; // 日
        g_timeinfo.weekday = g_rxBuffer[4]; // 星期
        g_timeinfo.hour    = g_rxBuffer[5]; // 時
        g_timeinfo.minute  = g_rxBuffer[6]; // 分
        g_timeinfo.second  = g_rxBuffer[7]; // 秒
        g_rxBuffer[0]    = 0;             // 使用後清除
        g_timeDemand     = 0;             // 重置時間需求標誌
        return HAL_OK;
    }
    return HAL_ERROR;
}