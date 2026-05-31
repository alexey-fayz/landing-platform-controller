#include "control/pid.h"

static float Clamp(
        float value,
        float min,
        float max)
{
    if(value > max)
        return max;

    if(value < min)
        return min;

    return value;
}

void PID_Init(
        PID_t *pid,
        float kp,
        float ki,
        float kd,
        float output_min,
        float output_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral = 0.0f;
    pid->prev_error = 0.0f;

    pid->output_min = output_min;
    pid->output_max = output_max;

    pid->integral_min = output_min;
    pid->integral_max = output_max;
}

void PID_Reset(PID_t *pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

float PID_Update(
        PID_t *pid,
        float setpoint,
        float measurement,
        float dt)
{
    float error =
            setpoint -
            measurement;

    pid->integral +=
            error * dt;

    pid->integral =
            Clamp(
                pid->integral,
                pid->integral_min,
                pid->integral_max);

    float derivative =
            (error -
             pid->prev_error) /
            dt;

    float output =
            pid->kp * error +
            pid->ki * pid->integral +
            pid->kd * derivative;

    output =
            Clamp(
                output,
                pid->output_min,
                pid->output_max);

    pid->prev_error =
            error;

    return output;
}