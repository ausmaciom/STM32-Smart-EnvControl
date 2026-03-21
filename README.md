# STM32 Dual-Zone Climate Controller

基於 STM32F103 (Cortex-M3) 的雙區溫濕度自動控制系統。
本專案採用 C++ 物件導向設計與有限狀態機 (FSM) 架構，實現非阻塞式 (Non-blocking) 的硬體控制，並針對無 FPU 核心進行底層效能優化。預留 UART 介面以便未來擴充 IoT 模組。

## 系統架構與核心模組介紹 (Core Components)

本專案將硬體驅動與業務邏輯高度解耦，以下為核心 .cpp 模組的功能說明：

* **main.cpp**
    * 系統進入點：負責 STM32 基礎時脈 (Clock)、硬體周邊 (I2C, GPIO, UART, RTC) 的 HAL 初始化。
    * 相依性注入：實作物件實例化，並將硬體指標傳遞給核心狀態機 `Loop` 啟動主迴圈。

* **Loop.cpp**
    * 核心狀態機 (FSM)：掌控系統全局邏輯。管理 MONITORING, DECIDE, VENT, HUMIDIFY, SYS_ERROR 等狀態切換。
    * 自動化決策：整合感測器數據進行判斷，內建遲滯區間 (Hysteresis) 防護機制，並處理來自 UART 的 IoT 強制覆蓋指令。

* **SHTC3.cpp**
    * 溫濕度感測器驅動：實作 I2C 通訊與嚴格的資料 CRC 校驗。
    * 效能優化：針對無 FPU 的 MCU，將原廠浮點數解析公式轉換為定點數運算 (Fixed-point math)，大幅提升執行效率；並內建軟體限流 (Rate-limiting) 防止 I2C 總線頻繁讀取卡死。

* **fan.cpp**
    * 排風扇控制：封裝 GPIO 腳位控制邏輯。
    * 安全機制：結合 SysTick (HAL_GetTick) 實作非阻塞式的超時關閉 (Timeout) 功能，避免系統因意外無限制運轉。

* **humidifier.cpp**
    * 加濕器控制：負責加濕模組的啟停控制。
    * 硬體防護：實作冷卻鎖定 (Cooldown) 機制，防止繼電器或模組在臨界值附近頻繁開關導致損壞。
