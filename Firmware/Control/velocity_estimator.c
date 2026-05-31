#include "control/velocity_estimator.h"
#include "control/lowpass_filter.h"

#include "main.h"

#include <math.h>
#include <corecrt_math_defines.h>

#define ENCODER_COUNTS      4096.0f

static VelocityEstimator_t estimator;

static LowPassFilter_t velocity_filter;

void VelocityEstimator_Init(void)
{
    estimator.velocity_rad_s = 0.0f;
    estimator.velocity_deg_s = 0.0f;

    estimator.filtered_velocity_rad_s = 0.0f;
    estimator.filtered_velocity_deg_s = 0.0f;

    estimator.previous_position = 0;

    estimator.previous_time = HAL_GetTick();

    estimator.initialized = 0;

    LPF_Init(
            &velocity_filter,
            20.0f,
            0.001f);
}
uint32_t current_time = HAL_GetTick();

void VelocityEstimator_Update(
        uint16_t position_raw)
{

    if(!estimator.initialized)
    {
        estimator.previous_position =
                position_raw;

        estimator.previous_time =
                current_time;

        estimator.initialized = 1;

        return;
    }

    float dt =
        ((float)(current_time -
                 estimator.previous_time))
        / 1000.0f;

    if(dt <= 0.0f)
    {
        return;
    }

    int32_t delta =
        (int32_t)position_raw -
        (int32_t)estimator.previous_position;

    if(delta > 2048)
        delta -= 4096;

    if(delta < -2048)
        delta += 4096;

    float velocity_rad =
        ((float)delta)
        * (2.0f * (float)M_PI)
        / ENCODER_COUNTS
        / dt;

    estimator.velocity_rad_s =
            velocity_rad;

    estimator.velocity_deg_s =
            velocity_rad *
            57.2957795f;

    estimator.filtered_velocity_rad_s =
            LPF_Update(
                &velocity_filter,
                velocity_rad);

    estimator.filtered_velocity_deg_s =
            estimator.filtered_velocity_rad_s *
            57.2957795f;

    estimator.previous_position =
            position_raw;

    estimator.previous_time =
            current_time;
}

float VelocityEstimator_GetVelocityRad(void)
{
    return estimator.velocity_rad_s;
}

float VelocityEstimator_GetVelocityDeg(void)
{
    return estimator.velocity_deg_s;
}

float VelocityEstimator_GetFilteredVelocityRad(void)
{
    return estimator.filtered_velocity_rad_s;
}

float VelocityEstimator_GetFilteredVelocityDeg(void)
{
    return estimator.filtered_velocity_deg_s;
}

VelocityEstimator_t*
VelocityEstimator_GetHandle(void)
{
    return &estimator;
}