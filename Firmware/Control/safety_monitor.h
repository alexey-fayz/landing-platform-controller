#ifndef SAFETY_MONITOR_H
#define SAFETY_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
    SAFETY_OK = 0,

    SAFETY_OVERCURRENT,

    SAFETY_OVERVOLTAGE,

    SAFETY_UNDERVOLTAGE,

    SAFETY_ENCODER_FAULT,

    SAFETY_OVERSPEED

} SafetyState_t;

typedef struct
{
    float bus_voltage;

    float motor_current;

    float velocity_deg_s;

    uint32_t encoder_timeout_ms;

    SafetyState_t state;

} SafetyMonitor_t;

void SafetyMonitor_Init(void);

void SafetyMonitor_Update(void);

SafetyState_t SafetyMonitor_GetState(void);

uint8_t SafetyMonitor_IsFault(void);

SafetyMonitor_t*
SafetyMonitor_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif