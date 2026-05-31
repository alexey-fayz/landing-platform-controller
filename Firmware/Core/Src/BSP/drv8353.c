#include "bsp/drv8353.h"

extern TIM_HandleTypeDef htim1;

static DRV8353_t drv;

/* --------------------------------
 * Внутренние функции
 * -------------------------------- */

static uint32_t DutyToCCR(float duty)
{
    if(duty < 0.0f)
    {
        duty = 0.0f;
    }

    if(duty > 1.0f)
    {
        duty = 1.0f;
    }

    return (uint32_t)(duty * PWM_TIMER_PERIOD);
}

/* --------------------------------
 * Init
 * -------------------------------- */

HAL_StatusTypeDef DRV8353_Init(void)
{
    drv.enabled = false;

    drv.duty_a = 0.0f;
    drv.duty_b = 0.0f;
    drv.duty_c = 0.0f;

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_1,
        0);

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_2,
        0);

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_3,
        0);

    return HAL_OK;
}

/* --------------------------------
 * Enable
 * -------------------------------- */

HAL_StatusTypeDef DRV8353_Enable(void)
{
    drv.enabled = true;

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

    return HAL_OK;
}

/* --------------------------------
 * Disable
 * -------------------------------- */

HAL_StatusTypeDef DRV8353_Disable(void)
{
    drv.enabled = false;

    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

    return HAL_OK;
}

/* --------------------------------
 * Запись ШИМ
 * -------------------------------- */

HAL_StatusTypeDef DRV8353_SetDutyABC(
    float duty_a,
    float duty_b,
    float duty_c)
{
    drv.duty_a = duty_a;
    drv.duty_b = duty_b;
    drv.duty_c = duty_c;

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_1,
        DutyToCCR(duty_a));

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_2,
        DutyToCCR(duty_b));

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_3,
        DutyToCCR(duty_c));

    return HAL_OK;
}

/* --------------------------------
 * FOC совместимость
 * -------------------------------- */

HAL_StatusTypeDef DRV8353_SetPhaseVoltage(
    float Ua,
    float Ub,
    float Uc)
{
    return DRV8353_SetDutyABC(
        Ua,
        Ub,
        Uc);
}

DRV8353_t* DRV8353_GetHandle(void)
{
    return &drv;
}