#ifndef VELOCITY_ESTIMATOR_H
#define VELOCITY_ESTIMATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
    float velocity_rad_s;
    float velocity_deg_s;

    float filtered_velocity_rad_s;
    float filtered_velocity_deg_s;

    uint16_t previous_position;

    uint32_t previous_time;

    uint8_t initialized;

} VelocityEstimator_t;

void VelocityEstimator_Init(void);

void VelocityEstimator_Update(
        uint16_t position_raw);

float VelocityEstimator_GetVelocityRad(void);

float VelocityEstimator_GetVelocityDeg(void);

float VelocityEstimator_GetFilteredVelocityRad(void);

float VelocityEstimator_GetFilteredVelocityDeg(void);

VelocityEstimator_t*
VelocityEstimator_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif