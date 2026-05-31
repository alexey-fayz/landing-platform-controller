#ifndef BSP_DRV8353_H
#define BSP_DRV8353_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ----------------------------------
 * Параметры TIM1
 * ---------------------------------- */

#define PWM_TIMER_PERIOD     3999U

typedef struct
{
    bool enabled;

    float duty_a;
    float duty_b;
    float duty_c;

} DRV8353_t;

/* ----------------------------------
 * API
 * ---------------------------------- */

HAL_StatusTypeDef DRV8353_Init(void);

HAL_StatusTypeDef DRV8353_Enable(void);

HAL_StatusTypeDef DRV8353_Disable(void);

HAL_StatusTypeDef DRV8353_SetDutyABC(
    float duty_a,
    float duty_b,
    float duty_c);

HAL_StatusTypeDef DRV8353_SetPhaseVoltage(
    float Ua,
    float Ub,
    float Uc);

DRV8353_t* DRV8353_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif