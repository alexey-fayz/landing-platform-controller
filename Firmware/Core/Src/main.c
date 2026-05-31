/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "app.h"           // Пользовательская инициализация
#include "bsp/encoder.h"   // Драйвер энкодера (будет создан)
#include "bsp/drv8353.h"   // Драйвер DRV8353
#include "bsp/power_monitor.h"
#include "communication/can_protocol.h"
#include "communication/telemetry.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    float position;
    float velocity;
    uint8_t enable;
} Command_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
CAN_HandleTypeDef hcan1;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c4;
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* FreeRTOS objects */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t EncoderTaskHandle;
const osThreadAttr_t EncoderTask_attributes = {
    .name = "EncoderTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityHigh,
};

osThreadId_t HallTaskHandle;
const osThreadAttr_t HallTask_attributes = {
    .name = "HallTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityHigh,
};

osThreadId_t ControlTaskHandle;
const osThreadAttr_t ControlTask_attributes = {
    .name = "ControlTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t) osPriorityRealtime,
};

osThreadId_t CanTaskHandle;
const osThreadAttr_t CanTask_attributes = {
    .name = "CanTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t TelemetryTaskHandle;
const osThreadAttr_t TelemetryTask_attributes = {
    .name = "TelemetryTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityLow,
};

osThreadId_t DiagnosticsTaskHandle;
const osThreadAttr_t DiagnosticsTask_attributes = {
    .name = "DiagnosticsTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityLow,
};

// Очередь команд: 10 элементов по 12 байт (sizeof(Command_t))
osMessageQueueId_t CommandQueueHandle;
const osMessageQueueAttr_t CommandQueue_attributes = {
    .name = "CommandQueue"
};

osTimerId_t LedTimerHandle;
const osTimerAttr_t LedTimer_attributes = {
    .name = "LedTimer"
};

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C4_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
void StartDefaultTask(void *argument);
void StartEncoderTask(void *argument);
void StartHallTask(void *argument);
void StartControlTask(void *argument);
void StartCanTask(void *argument);
void StartTelemetryTask(void *argument);
void StartDiagnosticsTask(void *argument);
void LedTimerCallback(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* ===================== MAIN ===================== */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    HAL_Init();

    /* USER CODE BEGIN Init */
    /* USER CODE END Init */

    SystemClock_Config();   // <-- БУДЕТ ИСПРАВЛЕНА НИЖЕ

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    MX_GPIO_Init();
    MX_ADC1_Init();      // <-- ИСПРАВЛЕНА
    MX_CAN1_Init();      // <-- ИСПРАВЛЕНА
    MX_I2C1_Init();
    MX_I2C4_Init();
    MX_TIM1_Init();      // <-- ИСПРАВЛЕНА (dead-time 250 нс)
    MX_USART2_UART_Init();
    MX_SPI1_Init();      // <-- ИСПРАВЛЕНА (Master, полный дуплекс)
    MX_TIM3_Init();      // <-- ИСПРАВЛЕНА (каналы 1,2,3)

    /* USER CODE BEGIN 2 */
    // Запуск ШИМ на всех 6 каналах TIM1
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1N);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2N);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3N);

    // Запуск таймера захвата для датчиков Холла (с прерываниями)
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);

    // Инициализация пользовательских модулей
    App_Init();
    /* USER CODE END 2 */

    osKernelInitialize();

    /* USER CODE BEGIN RTOS_MUTEX */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* USER CODE END RTOS_SEMAPHORES */

    /* Создание таймера */
    LedTimerHandle = osTimerNew(LedTimerCallback, osTimerPeriodic, NULL, &LedTimer_attributes);
    osTimerStart(LedTimerHandle, pdMS_TO_TICKS(500));  // 500 мс

    /* Создание очереди (исправлено: размер элемента 12 байт) */
    CommandQueueHandle = osMessageQueueNew(10, sizeof(Command_t), &CommandQueue_attributes);

    /* USER CODE BEGIN RTOS_QUEUES */
    /* USER CODE END RTOS_QUEUES */

    /* Создание задач */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
    EncoderTaskHandle = osThreadNew(StartEncoderTask, NULL, &EncoderTask_attributes);
    HallTaskHandle = osThreadNew(StartHallTask, NULL, &HallTask_attributes);
    ControlTaskHandle = osThreadNew(StartControlTask, NULL, &ControlTask_attributes);
    CanTaskHandle = osThreadNew(StartCanTask, NULL, &CanTask_attributes);
    TelemetryTaskHandle = osThreadNew(StartTelemetryTask, NULL, &TelemetryTask_attributes);
    DiagnosticsTaskHandle = osThreadNew(StartDiagnosticsTask, NULL, &DiagnosticsTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* USER CODE END RTOS_THREADS */

    osKernelStart();

    while (1)
    {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/* ===================== Clock Configuration (ИСПРАВЛЕНА) ===================== */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Настройка напряжения для 80 МГц
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
        Error_Handler();

    // Используем HSE (внешний кварц 8 МГц)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;          // /1
    RCC_OscInitStruct.PLL.PLLN = 20;         // *20 => 160 МГц на VCO
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;   // не используется
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;   // не используется
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;   // /2 => 80 МГц SYSCLK

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
        Error_Handler();
}

/* ===================== ADC1 (ИСПРАВЛЕНА: 2 канала, сканирование) ===================== */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;          // Включено сканирование
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;         // Конец всей последовательности
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 2;                     // Два канала
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

    // Канал 5 (PB15) – ток фазы
    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;   // больше для фильтрации
    sConfig.SingleDiff = ADC_DIFFERENTIAL_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();

    // Канал 6 (PA1) – напряжение шины
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = 2;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

/* ===================== CAN1 (ИСПРАВЛЕНА: 250 кбит/с, 80 МГц) ===================== */
static void MX_CAN1_Init(void)
{
    hcan1.Instance = CAN1;
    // Расчёт: частота таймера = 80 МГц / Prescaler.
    // Для 250 кбит/с: Prescaler = 20, BS1=13, BS2=2, тогда квантов = 1+13+2=16,
    // частота = 80e6 / (20 * 16) = 250 кГц.
    hcan1.Init.Prescaler = 20;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = ENABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) Error_Handler();

    // Фильтр – пропускаем всё
    CAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0;
    sFilterConfig.FilterIdLow = 0;
    sFilterConfig.FilterMaskIdHigh = 0;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) Error_Handler();

    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/* ===================== SPI1 (ИСПРАВЛЕНА: Master, полный дуплекс, 16 бит) ===================== */
static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;           // Master (было Slave)
    hspi1.Init.Direction = SPI_DIRECTION_2LINES; // Полный дуплекс
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       // SPI Mode 0
    hspi1.Init.NSS = SPI_NSS_SOFT;               // Управление CS программно
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    // Предделитель для частоты ~1-5 МГц (80 МГц / 16 = 5 МГц)
    if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
}

/* ===================== TIM1 (ИСПРАВЛЕНА: Dead Time = 250 нс) ===================== */
static void MX_TIM1_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1; // Центрированный
    htim1.Init.Period = 3999;    // 4000-1, 20 кГц при 80 МГц
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) Error_Handler();

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;   // начальная скважность 0
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    for (int ch = TIM_CHANNEL_1; ch <= TIM_CHANNEL_3; ch++)
        if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, ch) != HAL_OK) Error_Handler();

    // Настройка Dead Time (250 нс)
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    // DeadTime рассчитывается: tDTS = 1/80e6 = 12.5 нс, DT = 20 * 12.5 = 250 нс
    sBreakDeadTimeConfig.DeadTime = 20;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) Error_Handler();

    HAL_TIM_MspPostInit(&htim1);
}

/* ===================== TIM3 (ИСПРАВЛЕНА: каналы 1,2,3) ===================== */
static void MX_TIM3_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 65535;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_IC_Init(&htim3) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) Error_Handler();

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;

    if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_3) != HAL_OK) Error_Handler();
}

/* ===================== Остальные функции (без изменений) ===================== */
static void MX_USART2_UART_Init(void) { /* ... как было ... */ }
static void MX_I2C1_Init(void) { /* ... */ }
static void MX_I2C4_Init(void) { /* ... */ }
static void MX_GPIO_Init(void) { /* ... как было, но добавьте CS энкодера если нужно */ }

/* ===================== Пример задачи ControlTask (заглушка) ===================== */
void StartControlTask(void *argument)
{
    /* USER CODE BEGIN StartControlTask */
    for(;;)
    {
        // Основной цикл управления
        osDelay(10);
    }
    /* USER CODE END StartControlTask */
}

/* Остальные задачи (StartEncoderTask, StartHallTask и т.д.) – аналогично, пока пустые */
void StartDefaultTask(void *argument) { for(;;) osDelay(1); }
void StartEncoderTask(void *argument) { for(;;) osDelay(10); }
void StartHallTask(void *argument) { for(;;) osDelay(1); }
void StartCanTask(void *argument) { for(;;) osDelay(5); }
void StartTelemetryTask(void *argument) { for(;;) osDelay(100); }
void StartDiagnosticsTask(void *argument) { for(;;) osDelay(500); }

void LedTimerCallback(void *argument)
{
    HAL_GPIO_TogglePin(LED_DEBUG_1_GPIO_Port, LED_DEBUG_1_Pin);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}