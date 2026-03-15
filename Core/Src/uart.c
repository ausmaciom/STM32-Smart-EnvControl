#include "uart.h"

void initUART(void)
{
    g_commStatus = CommStatus::IDLE;
    txBuffer.startMarker = 0xAA;
    txBuffer.ackStatus   = (uint8_t)CommStatus::IDLE;
    g_uartReady            = 1;
}

static uint16_t convertFloatToInt(float value)
{
    return (uint16_t)(value * 10.0f);
}

void sendSensorDataBinary(float hotTemp, float hotHumid, float coldTemp, float coldHumid)
{
    if (g_uartReady != 1) return;

    txBuffer.hotTemp = convertFloatToInt(hotTemp);
    txBuffer.hotHumid = convertFloatToInt(hotHumid);
    txBuffer.coldTemp = convertFloatToInt(coldTemp);
    txBuffer.coldHumid = convertFloatToInt(coldHumid);
    txBuffer.ackStatus = (uint8_t)g_commStatus;

    if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)&g_txBuffer, sizeof(g_txBuffer)) == HAL_OK) {
        g_uartReady = 1; // 傳輸開始，設置標誌為 false
    } else {
        g_uartReady = 0; // 傳輸失敗，保持標誌為 true
    }
}

HAL_StatusTypeDef isRxMessageOK()
{
    if (rxBuffer[0] != 0xAA) return HAL_ERROR;

    if (rxBuffer[1] == 0x01)
    {
        g_timeinfo.year = g_rxBuffer[1]; // 年
        g_timeinfo.month = g_rxBuffer[2]; // 月
        g_timeinfo.day   = g_rxBuffer[3]; // 日
        g_timeinfo.weekday = g_rxBuffer[4]; // 星期
        g_timeinfo.hour    = g_rxBuffer[5]; // 時
        g_timeinfo.minute  = g_rxBuffer[6]; // 分
        g_timeinfo.second  = g_rxBuffer[7]; // 秒
        txBuffer.timeRequest     = 0; // 重置時間需求標誌
        return HAL_OK;
    }
    else if (rxBuffer[1] == 0x02)
    {
        g_iotCommand = g_rxBuffer[2];
        g_commStatus = CommStatus::IDLE; // 設置已收到iot command
        return HAL_OK;
    }
    else
        return HAL_ERROR;
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        memcpy(localRxBuffer, rxBuffer, RX_BUFFER_SIZE);
    }
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        g_uartReady = true; // 傳輸完成，重置標誌為 true
    }
}