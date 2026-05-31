/**
  * @file    app.h
  * @brief   Центральная инициализация приложения и глобальные объявления.
  */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"   // для очередей FreeRTOS

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  Глобальное состояние системы
  */
typedef struct {
    float current_angle;         // текущий угол с энкодера (градусы)
    float target_angle;          // целевой угол (из CAN или консоли)
    float current_speed;         // текущая скорость (градусы/с)
    float motor_current;         // ток двигателя (А)
    float bus_voltage;           // напряжение шины (В)
    uint8_t system_enabled;      // 1 – привод включён
    uint8_t error_flags;         // битовые флаги ошибок
} SystemState_t;

/* Exported variables --------------------------------------------------------*/
extern osMessageQueueId_t CommandQueueHandle;   // объявлена в main.c
extern SystemState_t g_system;                  // глобальное состояние

/* Exported functions prototypes ---------------------------------------------*/
void App_Init(void);                            // инициализация всех модулей
void App_ErrorHandler(uint8_t error_code);      // централизованная обработка ошибок

/* Колбэки, вызываемые из прерываний (реализованы в app.c) */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */