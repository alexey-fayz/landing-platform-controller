/**
  * @file    uart_console.c
  * @brief   Реализация консольного ввода через UART.
  */

#include "uart_console.h"
#include "telemetry.h"
#include "app.h"
#include "cmsis_os.h"

static UART_HandleTypeDef *huart_ptr = NULL;
static char rx_buffer[64];
static uint16_t rx_index = 0;
static uint8_t rx_byte;

extern osMessageQueueId_t CommandQueueHandle;

void UART_Console_Init(UART_HandleTypeDef *huart)
{
    huart_ptr = huart;
    HAL_UART_Receive_IT(huart_ptr, &rx_byte, 1);
}

void UART_Console_RxCallback(void)
{
    if (huart_ptr == NULL) return;
    if (rx_index < sizeof(rx_buffer) - 1) {
        rx_buffer[rx_index++] = rx_byte;
        if (rx_byte == '\r' || rx_byte == '\n') {
            rx_buffer[rx_index - 1] = '\0';
        }
    }
    HAL_UART_Receive_IT(huart_ptr, &rx_byte, 1);
}

void UART_Console_Process(void)
{
    if (rx_index == 0) return;

    char cmd[64];
    uint16_t len = rx_index;
    if (len > sizeof(cmd)-1) len = sizeof(cmd)-1;
    memcpy(cmd, rx_buffer, len);
    cmd[len] = '\0';
    rx_index = 0;

    if (strncmp(cmd, "enable", 6) == 0) {
        g_system.system_enabled = 1;
        TELEM_SendString("System enabled\r\n");
    }
    else if (strncmp(cmd, "disable", 7) == 0) {
        g_system.system_enabled = 0;
        TELEM_SendString("System disabled\r\n");
    }
    else if (strncmp(cmd, "angle ", 6) == 0) {
        float angle = atof(cmd + 6);
        Command_t cmd_struct;
        cmd_struct.position = angle;
        cmd_struct.velocity = 0;
        cmd_struct.enable = g_system.system_enabled;
        if (CommandQueueHandle != NULL) {
            osMessageQueuePut(CommandQueueHandle, &cmd_struct, 0, 0);
        }
        TELEM_SendFormatted("Set angle: %.2f\r\n", angle);
    }
    else if (strcmp(cmd, "status") == 0) {
        TELEM_SendFormatted("Angle: %.2f  Target: %.2f  Speed: %.1f  Enabled: %d  Error: %d\r\n",
                            g_system.current_angle, g_system.target_angle,
                            g_system.current_speed, g_system.system_enabled,
                            g_system.error_flags);
    }
    else if (strlen(cmd) > 0) {
        TELEM_SendString("Unknown command\r\n");
    }
}