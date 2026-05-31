/**
  * @file    power_monitor.h
  * @brief   Драйвер датчика тока/напряжения INA219 (I2C).
  */

#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief  Инициализация INA219.
  * @param  hi2c : указатель на I2C_HandleTypeDef (например, &hi2c1).
  */
void PWR_Init(I2C_HandleTypeDef *hi2c);

/**
  * @brief  Получение тока в амперах.
  * @return ток (А).
  */
float PWR_GetCurrent(void);

/**
  * @brief  Получение напряжения шины (BUS) в вольтах.
  * @return напряжение (В).
  */
float PWR_GetVoltage(void);

/**
  * @brief  Получение мощности (Вт) = ток * напряжение.
  * @return мощность (Вт).
  */
float PWR_GetPower(void);

/**
  * @brief  Калибровка INA219 (настройка под шунт).
  * @param  current_range   : максимальный ожидаемый ток (А)
  * @param  shunt_resistor   : сопротивление шунта (Ом)
  */
void PWR_Calibrate(float current_range, float shunt_resistor);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MONITOR_H */