#include "communication/telemetry.h"

#include "control/motor_controller.h"
#include "control/velocity_estimator.h"
#include "control/safety_monitor.h"

#include "bsp/encoder.h"

#include "main.h"

#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

static char telemetry_buffer[256];

void Telemetry_Init(void)
{
    memset(
        telemetry_buffer,
        0,
        sizeof(telemetry_buffer));
}

void Telemetry_Send(void)
{
    MotorController_t* motor =
            MotorController_GetHandle();

    SafetyMonitor_t* safety =
            SafetyMonitor_GetHandle();

    int len =
        snprintf(
            telemetry_buffer,
            sizeof(telemetry_buffer),

            "POS=%.2f "
            "VEL=%.2f "
            "TARGET=%.2f "
            "FAULT=%d\r\n",

            motor->current_position_deg,

            VelocityEstimator_GetFilteredVelocityDeg(),

            motor->target_position_deg,

            safety->state);

    HAL_UART_Transmit(
            &huart2,
            (uint8_t*)telemetry_buffer,
            len,
            100);
}