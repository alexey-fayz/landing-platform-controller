#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ---------------------------
 * Константы энкодера
 * --------------------------- */
#define ENCODER_RESOLUTION        4096U
#define ENCODER_MAX_COUNT         4095U

#define ENCODER_MECH_ANGLE_SCALE  (360.0f / 4096.0f)

/* ---------------------------
 * Структура состояния
 * --------------------------- */
typedef struct
{
    uint16_t raw_position;
    float mechanical_angle_deg;
    float mechanical_angle_rad;
    float velocity_rad_s;

    uint16_t raw_prev;
    uint32_t timestamp_prev;

    bool initialized;

} Encoder_t;

/* ---------------------------
 * API
 * --------------------------- */

HAL_StatusTypeDef Encoder_Init(void);

HAL_StatusTypeDef Encoder_ReadRaw(uint16_t *position);

HAL_StatusTypeDef Encoder_Update(void);

uint16_t Encoder_GetRaw(void);

float Encoder_GetAngleDeg(void);

float Encoder_GetAngleRad(void);

float Encoder_GetVelocity(void);

Encoder_t* Encoder_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif