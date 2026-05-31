/**
  * @file    power_monitor.c
  * @brief   Реализация драйвера INA219.
  */

#include "power_monitor.h"

static I2C_HandleTypeDef *hi2c_ptr = NULL;
static uint16_t calibration_value = 0;
static float current_lsb = 0.0f;

// Регистры INA219
#define INA219_REG_CONFIG       0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE   0x02
#define INA219_REG_POWER        0x03
#define INA219_REG_CURRENT      0x04
#define INA219_REG_CALIBRATION  0x05

// Настройки по умолчанию: 32 выборки, 12 бит, 2 мс
#define INA219_CONFIG_DEFAULT   0x399F  // см. даташит

/* Приватные функции */
static uint16_t read_register(uint8_t reg);
static void write_register(uint8_t reg, uint16_t value);

void PWR_Init(I2C_HandleTypeDef *hi2c)
{
    hi2c_ptr = hi2c;
    // Сброс (запись 0x8000 в конфиг)
    write_register(INA219_REG_CONFIG, 0x8000);
    HAL_Delay(10);
    // Запись конфигурации по умолчанию
    write_register(INA219_REG_CONFIG, INA219_CONFIG_DEFAULT);
    // Калибровка: по умолчанию 1 А, шунт 0.1 Ом
    PWR_Calibrate(1.0f, 0.1f);
}

void PWR_Calibrate(float current_range, float shunt_resistor)
{
    // current_lsb = максимальный ток / 32768 (15 бит)
    current_lsb = current_range / 32768.0f;
    // Calibration register = 0.04096 / (current_lsb * shunt_resistor)
    uint16_t cal = (uint16_t)(0.04096f / (current_lsb * shunt_resistor));
    write_register(INA219_REG_CALIBRATION, cal);
    calibration_value = cal;
}

float PWR_GetCurrent(void)
{
    int16_t raw_current = (int16_t)read_register(INA219_REG_CURRENT);
    return (float)raw_current * current_lsb;
}

float PWR_GetVoltage(void)
{
    uint16_t raw_voltage = read_register(INA219_REG_BUSVOLTAGE);
    // напряжение в вольтах: (raw >> 3) * 4 мВ / 1000 = В
    float voltage = (float)(raw_voltage >> 3) * 0.004f;
    return voltage;
}

float PWR_GetPower(void)
{
    uint16_t raw_power = read_register(INA219_REG_POWER);
    // мощность: raw_power * 20 * current_lsb
    return (float)raw_power * 20.0f * current_lsb;
}

static uint16_t read_register(uint8_t reg)
{
    uint8_t tx[1] = {reg};
    uint8_t rx[2] = {0};
    HAL_I2C_Master_Transmit(hi2c_ptr, INA219_I2C_ADDR << 1, tx, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(hi2c_ptr, INA219_I2C_ADDR << 1, rx, 2, HAL_MAX_DELAY);
    return ((uint16_t)rx[0] << 8) | rx[1];
}

static void write_register(uint8_t reg, uint16_t value)
{
    uint8_t tx[3];
    tx[0] = reg;
    tx[1] = (uint8_t)(value >> 8);
    tx[2] = (uint8_t)(value & 0xFF);
    HAL_I2C_Master_Transmit(hi2c_ptr, INA219_I2C_ADDR << 1, tx, 3, HAL_MAX_DELAY);
}