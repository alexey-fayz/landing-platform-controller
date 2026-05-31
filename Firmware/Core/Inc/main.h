/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/**
  * @brief  Структура команды для управления приводом (через очередь FreeRTOS)
  */
typedef struct {
    float position;   // целевая позиция (градусы)
    float velocity;   // целевая скорость (градусы/с) – опционально
    uint8_t enable;   // 1 – включить привод, 0 – выключить
} Command_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

// -------------------- I2C (датчик тока INA219) --------------------
#define INA219_I2C_ADDR         0x40    // типовой адрес (A0=A1=GND)

// -------------------- Энкодер AS5311 (SSI) --------------------
#define ENC_CS_PIN              GPIO_PIN_6      // PB6
#define ENC_CS_PORT             GPIOB
#define ENC_SCK_PIN             GPIO_PIN_5      // PA5 (SPI1_SCK)
#define ENC_SCK_PORT            GPIOA
#define ENC_MISO_PIN            GPIO_PIN_11     // PA11 (SSI_MISO)
#define ENC_MISO_PORT           GPIOA

// -------------------- Драйвер DRV8353 (управляющие сигналы) ----------------
#define DRV_EN_PIN              GPIO_PIN_8      
#define DRV_EN_PORT             GPIOB
#define DRV_nFAULT_PIN          GPIO_PIN_9      
#define DRV_nFAULT_PORT         GPIOB
#define DRV_nSLEEP_PIN          GPIO_PIN_10     // PB10
#define DRV_nSLEEP_PORT         GPIOB

// -------------------- CAN протокол (PGN) --------------------
#define CAN_ID_TARGET_ANGLE     0x100   // приём: целевой угол (2 байта, градусы * 10)
#define CAN_ID_TELEMETRY        0x101   // передача: текущий угол, статус

// -------------------- ШИМ (TIM1) --------------------
#define PWM_PERIOD              3999    // 4000-1, частота 20 кГц при SYSCLK=80 МГц

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void App_Init(void);   // инициализация всех пользовательских модулей (BSP, Control, Communication)

/* USER CODE BEGIN EFP */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);   // колбэк для датчиков Холла
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_DEBUG_1_Pin         GPIO_PIN_13
#define LED_DEBUG_1_GPIO_Port   GPIOC
#define LED_DEBUG_2_Pin         GPIO_PIN_14
#define LED_DEBUG_2_GPIO_Port   GPIOC

#define HALL_A_Pin              GPIO_PIN_6
#define HALL_A_GPIO_Port        GPIOA
#define HALL_B_Pin              GPIO_PIN_7
#define HALL_B_GPIO_Port        GPIOA
#define HALL_C_Pin              GPIO_PIN_0
#define HALL_C_GPIO_Port        GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */