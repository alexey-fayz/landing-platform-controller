/**
  * @file    app.c
  * @brief   Реализация инициализации и колбэков приложения.
  */

#include "app.h"
#include "cmsis_os.h"
#include "tasks.h"

/* BSP модули (драйверы периферии) */
#include "bsp/encoder.h"
#include "bsp/hall.h"
#include "bsp/drv8353.h"
#include "bsp/power_monitor.h"

/* Алгоритмы управления */
#include "control/pid.h"
#include "control/position_controller.h"
#include "control/stabilization.h"

/* Коммуникации */
#include "communication/can_protocol.h"
#include "communication/telemetry.h"
#include "communication/uart_console.h"

/* Утилиты */
#include "utils/filters.h"
#include "utils/math_utils.h"

/* External variables from main.c (HAL handles) */
extern TIM_HandleTypeDef htim3;
extern I2C_HandleTypeDef hi2c1;
extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart2;

/* Private variables ---------------------------------------------------------*/
SystemState_t g_system = {
    .current_angle = 0.0f,
    .target_angle = 0.0f,
    .current_speed = 0.0f,
    .motor_current = 0.0f,
    .bus_voltage = 0.0f,
    .system_enabled = 0,
    .error_flags = 0
};

/* Private function prototypes -----------------------------------------------*/
static void System_Update(void);

/* Private static variables for filters */
static float filtered_current = 0.0f;
static float filtered_voltage = 0.0f;

/* Public functions ----------------------------------------------------------*/

void App_Init(void)
{
    // 1. Инициализация BSP (низкоуровневых драйверов)
    ENC_Init();                     // энкодер AS5311 (SPI/GPIO)
    HALL_Init(&htim3);              // датчики Холла (TIM3)
    DRV_Init();                     // драйвер DRV8353 (GPIO)
    PWR_Init(&hi2c1);               // датчик тока INA219 (I2C1)

    // 2. Инициализация алгоритмов управления
    STAB_Init(&g_stab, 0.01f);      // dt = 10 мс (период ControlTask)

    // 3. Инициализация коммуникационных модулей
    CAN_Init(&hcan1);               // CAN фильтры
    TELEM_Init(&huart2);            // телеметрия через UART
    UART_Console_Init(&huart2);     // консольные команды

    // 4. Запуск аппаратных модулей
    DRV_Enable();                   // разрешить работу драйвера
    CAN_Start();                    // активировать приём CAN

    // 5. Фильтры для АЦП (простая инициализация, alpha = 0.1)
    filtered_current = 0.0f;
    filtered_voltage = 0.0f;

    // 6. Установка начального состояния
    g_system.system_enabled = 1;
    g_system.error_flags = 0;

    // 7. Отладка
    TELEM_SendString("System initialized\r\n");
}

void App_ErrorHandler(uint8_t error_code)
{
    g_system.error_flags |= error_code;
    DRV_Disable();
    g_system.system_enabled = 0;

    // Мигаем светодиодом кодом ошибки (быстрое мигание)
    for (int i = 0; i < error_code; i++) {
        HAL_GPIO_TogglePin(LED_DEBUG_1_GPIO_Port, LED_DEBUG_1_Pin);
        osDelay(100);
        HAL_GPIO_TogglePin(LED_DEBUG_1_GPIO_Port, LED_DEBUG_1_Pin);
        osDelay(100);
    }

    TELEM_SendFormatted("Error: 0x%02X\r\n", error_code);
    while (1) {
        // Бесконечный цикл – требуется сброс
    }
}

/* Колбэки прерываний -------------------------------------------------------*/

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        HALL_IRQHandler();   // обновление состояния Холлов и скорости
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1) {
        CAN_ReceiveCallback();   // разбор и помещение в очередь CommandQueue
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        UART_Console_RxCallback();   // обработка введённой строки
    }
}

/* Private functions ---------------------------------------------------------*/

static void System_Update(void)
{
    // Чтение датчиков
    g_system.current_angle = ENC_GetAngleDeg();
    g_system.motor_current = PWR_GetCurrent();
    g_system.bus_voltage   = PWR_GetVoltage();

    // Фильтрация шумов (простой фильтр низких частот)
    filtered_current = filtered_current * 0.9f + g_system.motor_current * 0.1f;
    filtered_voltage = filtered_voltage * 0.9f + g_system.bus_voltage * 0.1f;

    // Получение целевого угла из очереди (есть ли команда)
    Command_t cmd;
    if (osMessageQueueGet(CommandQueueHandle, &cmd, NULL, 0) == osOK) {
        g_system.target_angle = cmd.position;
        g_system.system_enabled = cmd.enable;
    }

    // Обновление скорости по Холлам (если реализовано)
    g_system.current_speed = HALL_GetSpeedRPM() / 60.0f * 360.0f;

    // Передача в регулятор стабилизации
    STAB_Update(&g_stab, g_system.target_angle, g_system.current_angle,
                g_system.current_speed, filtered_current);

    // Отправка телеметрии по CAN и UART (с периодом, чтобы не забивать)
    static uint32_t last_telemetry = 0;
    uint32_t now = xTaskGetTickCount();
    if ((now - last_telemetry) > pdMS_TO_TICKS(50)) {
        last_telemetry = now;
        CAN_SendTelemetry(g_system.current_angle, g_system.current_speed,
                          g_system.system_enabled, g_system.error_flags);
        TELEM_SendStatus(g_system.current_angle, g_system.target_angle,
                         filtered_current, filtered_voltage);
    }
}

/* Эта функция вызывается из ControlTask (tasks.c) */
void System_UpdateFromTask(void)
{
    System_Update();
}