/**
  * @file    hall.c
  * @brief   Реализация драйвера датчиков Холла.
  */

#include "hall.h"

static TIM_HandleTypeDef *htim3_ptr = NULL;
static volatile uint8_t hall_state = 0;
static volatile int32_t hall_transitions = 0;
static volatile uint32_t last_capture_time = 0;

/**
  * @brief  Получение текущего состояния (читаем напрямую из GPIO).
  */
static uint8_t read_hall_gpio(void)
{
    uint8_t state = 0;
    if (HAL_GPIO_ReadPin(HALL_A_GPIO_Port, HALL_A_Pin)) state |= 0x01;
    if (HAL_GPIO_ReadPin(HALL_B_GPIO_Port, HALL_B_Pin)) state |= 0x02;
    if (HAL_GPIO_ReadPin(HALL_C_GPIO_Port, HALL_C_Pin)) state |= 0x04;
    return state;
}

void HALL_Init(TIM_HandleTypeDef *htim)
{
    htim3_ptr = htim;
    // Начальное состояние
    hall_state = read_hall_gpio();
    hall_transitions = 0;
    last_capture_time = 0;
}

uint8_t HALL_GetState(void)
{
    return hall_state;
}

int32_t HALL_GetTransitionCount(void)
{
    return hall_transitions;
}

void HALL_ResetTransitionCount(void)
{
    hall_transitions = 0;
}

/**
  * @brief  Вызывается из прерывания TIM3 при каждом захвате.
  *         Обновляет состояние и увеличивает счётчик переходов.
  */
void HALL_IRQHandler(void)
{
    // Определяем новое состояние
    uint8_t new_state = read_hall_gpio();
    if (new_state != hall_state) {
        hall_state = new_state;
        hall_transitions++;
    }
}

float HALL_GetSpeedRPM(void)
{
    // Примерный расчёт: за время между вызовами (в задаче) было hall_transitions
    // Каждый механический оборот даёт 6 переходов (3 датчика, 2 фронта? Уточнить)
    // Здесь упрощённо: скорость будет считаться в задаче по формуле.
    // Возвращаем последнее значение или вызываем внешний расчёт.
    // Для простоты возвращаем 0, т.к. скорость рассчитывается в tasks.c.
    return 0.0f;
}