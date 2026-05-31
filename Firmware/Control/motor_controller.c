#include "control/motor_controller.h"

#include "control/pid.h"

#include "bsp/encoder.h"
#include "bsp/drv8353.h"

#include <math.h>

static MotorController_t motor;

static PID_t position_pid;
static PID_t velocity_pid;

static float NormalizeDuty(float value)
{
    if(value > 1.0f)
        value = 1.0f;

    if(value < -1.0f)
        value = -1.0f;

    return value;
}

void MotorController_Init(void)
{
    motor.enabled = false;

    motor.target_position_deg = 0.0f;

    PID_Init(
            &position_pid,
            0.15f,
            0.0f,
            0.005f,
            -100.0f,
             100.0f);

    PID_Init(
            &velocity_pid,
            0.02f,
            0.001f,
            0.0f,
            -1.0f,
             1.0f);
}

void MotorController_Enable(void)
{
    motor.enabled = true;

    DRV8353_Enable();
}

void MotorController_Disable(void)
{
    motor.enabled = false;

    DRV8353_Disable();

    PID_Reset(&position_pid);
    PID_Reset(&velocity_pid);
}

void MotorController_SetPosition(
        float position_deg)
{
    motor.target_position_deg =
            position_deg;
}

void MotorController_Update(
        float dt)
{
    if(!motor.enabled)
        return;

    Encoder_Update();

    motor.current_position_deg =
            Encoder_GetAngleDeg();

    motor.current_velocity_deg_s =
            Encoder_GetVelocity() *
            57.2957795f;

    motor.target_velocity_deg_s =
            PID_Update(
                &position_pid,
                motor.target_position_deg,
                motor.current_position_deg,
                dt);

    motor.control_output =
            PID_Update(
                &velocity_pid,
                motor.target_velocity_deg_s,
                motor.current_velocity_deg_s,
                dt);

    float duty =
            NormalizeDuty(
                motor.control_output);

    float dutyA =
            0.5f +
            0.5f * duty;

    float dutyB =
            0.5f -
            0.5f * duty;

    float dutyC =
            0.5f;

    DRV8353_SetDutyABC(
            dutyA,
            dutyB,
            dutyC);
}

MotorController_t*
MotorController_GetHandle(void)
{
    return &motor;
}