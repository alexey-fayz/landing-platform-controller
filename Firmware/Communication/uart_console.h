/**
  * @file    uart_console.h
  * @brief   Простая консоль для ввода команд через UART.
  */

#ifndef UART_CONSOLE_H
#define UART_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief  Инициализация консоли.
  * @param  huart: указатель на UART_HandleTypeDef
  */
void UART_Console_Init(UART_HandleTypeDef *huart);

/**
  * @brief  Периодический вызов для обработки введённых данных.
  *         Вызывается из задачи TelemetryTask.
  */
void UART_Console_Process(void);

/**
  * @brief  Колбэк, вызываемый из прерывания UART при завершении приёма.
  */
void UART_Console_RxCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_CONSOLE_H */