/**
  * @file    hall.h
  * @brief   Драйвер датчиков Холла (3 канала, TIM3 Input Capture).
  */

#ifndef HALL_H
#define HALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief  Инициализация датчиков Холла.
  * @param  htim : указатель на TIM_HandleTypeDef (TIM3).
  */
void HALL_Init(TIM_HandleTypeDef *htim);

/**
  * @brief  Получение текущей комбинации датчиков (0..7).
  * @return битовая маска: (HALL_C << 2) | (HALL_B << 1) | HALL_A
  */
uint8_t HALL_GetState(void);

/**
  * @brief  Получение количества переходов (для расчёта скорости).
  * @return количество зарегистрированных изменений.
  */
int32_t HALL_GetTransitionCount(void);

/**
  * @brief  Сброс счётчика переходов.
  */
void HALL_ResetTransitionCount(void);

/**
  * @brief  Обработчик прерывания TIM3 (вызывается из stm32l4xx_it.c).
  */
void HALL_IRQHandler(void);

/**
  * @brief  Получение скорости вращения в об/мин (приблизительно).
  * @return скорость (RPM).
  */
float HALL_GetSpeedRPM(void);

#ifdef __cplusplus
}
#endif

#endif /* HALL_H */