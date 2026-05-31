#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    float target_position_deg;

    float current_position_deg;

    float target_velocity_deg_s;

    float current_velocity_deg_s;

    float control_output;

    bool enabled;

} MotorController_t;

void MotorController_Init(void);

void MotorController_Enable(void);

void MotorController_Disable(void);

void MotorController_SetPosition(
        float position_deg);

void MotorController_Update(
        float dt);

MotorController_t*
MotorController_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif