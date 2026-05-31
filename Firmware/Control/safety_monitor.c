#include "control/safety_monitor.h"

#include "control/velocity_estimator.h"

#include "bsp/drv8353.h"

#include "main.h"

#define MAX_BUS_VOLTAGE      28.0f
#define MIN_BUS_VOLTAGE      18.0f

#define MAX_MOTOR_CURRENT    10.0f

#define MAX_SPEED_DEG_S      1000.0f

static SafetyMonitor_t safety;

static float ReadBusVoltage(void)
{
    return safety.bus_voltage;
}

static float ReadMotorCurrent(void)
{
    return safety.motor_current;
}

void SafetyMonitor_Init(void)
{
    safety.bus_voltage = 24.0f;

    safety.motor_current = 0.0f;

    safety.velocity_deg_s = 0.0f;

    safety.encoder_timeout_ms = 0;

    safety.state = SAFETY_OK;
}

void SafetyMonitor_Update(void)
{
    safety.bus_voltage =
            ReadBusVoltage();

    safety.motor_current =
            ReadMotorCurrent();

    safety.velocity_deg_s =
            VelocityEstimator_GetFilteredVelocityDeg();

    safety.state =
            SAFETY_OK;

    if(safety.bus_voltage >
       MAX_BUS_VOLTAGE)
    {
        safety.state =
                SAFETY_OVERVOLTAGE;
    }

    if(safety.bus_voltage <
       MIN_BUS_VOLTAGE)
    {
        safety.state =
                SAFETY_UNDERVOLTAGE;
    }

    if(safety.motor_current >
       MAX_MOTOR_CURRENT)
    {
        safety.state =
                SAFETY_OVERCURRENT;
    }

    if(fabsf(safety.velocity_deg_s) >
       MAX_SPEED_DEG_S)
    {
        safety.state =
                SAFETY_OVERSPEED;
    }

    if(safety.state !=
       SAFETY_OK)
    {
        DRV8353_Disable();
    }
}

SafetyState_t
SafetyMonitor_GetState(void)
{
    return safety.state;
}

uint8_t
SafetyMonitor_IsFault(void)
{
    return
        (safety.state != SAFETY_OK);
}

SafetyMonitor_t*
SafetyMonitor_GetHandle(void)
{
    return &safety;
}