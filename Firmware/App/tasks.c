/**
  * @file    tasks.c
  * @brief   Реализация задач FreeRTOS для управления системой.
  */

#include "tasks.h"
#include "cmsis_os.h"
#include "app.h"

#include "bsp/encoder.h"
#include "bsp/hall.h"
#include "bsp/drv8353.h"
#include "bsp/power_monitor.h"

#include "control/pid.h"
#include "control/position_controller.h"
#include "control/stabilization.h"

#include "communication/can_protocol.h"
#include "communication/telemetry.h"
#include "communication/uart_console.h"

#include "utils/filters.h"
#include "utils/math_utils.h"

/* Периоды выполнения задач (мс) --------------------------------------------*/
#define ENCODER_PERIOD_MS       5
#define HALL_PERIOD_MS          5
#define CONTROL_PERIOD_MS       10
#define CAN_PERIOD_MS           10
#define TELEMETRY_PERIOD_MS     50
#define DIAGNOSTICS_PERIOD_MS   500

/* Внешние переменные -------------------------------------------------------*/
extern SystemState_t g_system;

/* Приватные прототипы ------------------------------------------------------*/
static void ProcessEncoder(void);
static void ProcessHall(void);
static void ProcessControl(void);
static void ProcessCAN(void);
static void ProcessTelemetry(void);
static void ProcessDiagnostics(void);

/* Инициализация (опционально) ----------------------------------------------*/
void Tasks_Init(void)
{
    // Ничего не требует, но можно добавить настройку приоритетов или семафоров
}

/* Реализация задач ---------------------------------------------------------*/

void StartDefaultTask(void *argument)
{
    for (;;) {
        osDelay(1000);
    }
}

void StartEncoderTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        ProcessEncoder();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ENCODER_PERIOD_MS));
    }
}

void StartHallTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        ProcessHall();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(HALL_PERIOD_MS));
    }
}

void StartControlTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        ProcessControl();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

void StartCanTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        ProcessCAN();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(CAN_PERIOD_MS));
    }
}

void StartTelemetryTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        ProcessTelemetry();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TELEMETRY_PERIOD_MS));
    }
}

void StartDiagnosticsTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        ProcessDiagnostics();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(DIAGNOSTICS_PERIOD_MS));
    }
}

/* Приватные функции (бизнес-логика) ----------------------------------------*/

static void ProcessEncoder(void)
{
    float angle = ENC_GetAngleDeg();
    angle = NormalizeAngle(angle);
    g_system.current_angle = angle;
}

static void ProcessHall(void)
{
    static uint32_t last_tick = 0;
    uint32_t now = xTaskGetTickCount();
    float delta_sec = (now - last_tick) / 1000.0f;

    if (delta_sec > 0.01f) {
        int32_t transitions = HALL_GetTransitionCount();
        float rpm = (transitions / 6.0f) / delta_sec * 60.0f;
        g_system.current_speed = rpm / 60.0f * 360.0f;
        HALL_ResetTransitionCount();
        last_tick = now;
    }

    if (g_system.system_enabled) {
        STAB_CommutationStep(HALL_GetState());
    }
}

static void ProcessControl(void)
{
    extern void System_Update(void);  // определена в app.c
    System_Update();
}

static void ProcessCAN(void)
{
    CAN_SendTelemetry(g_system.current_angle,
                      g_system.current_speed,
                      g_system.system_enabled,
                      g_system.error_flags);
}

static void ProcessTelemetry(void)
{
    TELEM_SendStatus(g_system.current_angle,
                     g_system.target_angle,
                     g_system.motor_current,
                     g_system.bus_voltage);
    UART_Console_Process();
}

static void ProcessDiagnostics(void)
{
    static uint8_t led = 0;
    if (g_system.system_enabled && g_system.error_flags == 0) {
        HAL_GPIO_TogglePin(LED_DEBUG_1_GPIO_Port, LED_DEBUG_1_Pin);
    } else {
        HAL_GPIO_WritePin(LED_DEBUG_1_GPIO_Port, LED_DEBUG_1_Pin,
                          led ? GPIO_PIN_SET : GPIO_PIN_RESET);
        led = !led;
    }

    if (g_system.bus_voltage < 20.0f) App_ErrorHandler(0x01);
    if (g_system.motor_current > 5.0f) App_ErrorHandler(0x02);
    if (DRV_IsFault()) App_ErrorHandler(0x04);
}