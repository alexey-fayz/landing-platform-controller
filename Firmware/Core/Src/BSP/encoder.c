#include "bsp/encoder.h"

#include <math.h>

extern SPI_HandleTypeDef hspi1;

static Encoder_t encoder;

/* ------------------------------------
 * Локальные функции
 * ------------------------------------ */

static uint32_t Encoder_GetTimestampMs(void)
{
    return HAL_GetTick();
}

/* ------------------------------------
 * Инициализация
 * ------------------------------------ */

HAL_StatusTypeDef Encoder_Init(void)
{
    encoder.raw_position = 0;
    encoder.raw_prev = 0;

    encoder.mechanical_angle_deg = 0.0f;
    encoder.mechanical_angle_rad = 0.0f;
    encoder.velocity_rad_s = 0.0f;

    encoder.timestamp_prev = Encoder_GetTimestampMs();
    encoder.initialized = true;

    return HAL_OK;
}

/* ------------------------------------
 * Чтение SSI
 * ------------------------------------ */

HAL_StatusTypeDef Encoder_ReadRaw(uint16_t *position)
{
    HAL_StatusTypeDef status;

    uint16_t tx_word = 0x0000;
    uint16_t rx_word = 0x0000;

    status = HAL_SPI_TransmitReceive(
                &hspi1,
                (uint8_t*)&tx_word,
                (uint8_t*)&rx_word,
                1,
                10);

    if(status != HAL_OK)
    {
        return status;
    }

    rx_word &= 0x0FFF;

    *position = rx_word;

    return HAL_OK;
}

/* ------------------------------------
 * Обновление данных
 * ------------------------------------ */

HAL_StatusTypeDef Encoder_Update(void)
{
    uint16_t raw;

    HAL_StatusTypeDef status;

    status = Encoder_ReadRaw(&raw);

    if(status != HAL_OK)
    {
        return status;
    }

    uint32_t now = Encoder_GetTimestampMs();

    float dt =
        ((float)(now - encoder.timestamp_prev)) / 1000.0f;

    int32_t delta =
        (int32_t)raw -
        (int32_t)encoder.raw_prev;

    if(delta > 2048)
    {
        delta -= 4096;
    }

    if(delta < -2048)
    {
        delta += 4096;
    }

    if(dt > 0.0f)
    {
        encoder.velocity_rad_s =
            ((float)delta) *
            (2.0f * (float)M_PI / 4096.0f) /
            dt;
    }

    encoder.raw_prev = raw;
    encoder.timestamp_prev = now;

    encoder.raw_position = raw;

    encoder.mechanical_angle_deg =
        raw * ENCODER_MECH_ANGLE_SCALE;

    encoder.mechanical_angle_rad =
        encoder.mechanical_angle_deg *
        ((float)M_PI / 180.0f);

    return HAL_OK;
}

/* ------------------------------------
 * Getters
 * ------------------------------------ */

uint16_t Encoder_GetRaw(void)
{
    return encoder.raw_position;
}

float Encoder_GetAngleDeg(void)
{
    return encoder.mechanical_angle_deg;
}

float Encoder_GetAngleRad(void)
{
    return encoder.mechanical_angle_rad;
}

float Encoder_GetVelocity(void)
{
    return encoder.velocity_rad_s;
}

Encoder_t* Encoder_GetHandle(void)
{
    return &encoder;
}