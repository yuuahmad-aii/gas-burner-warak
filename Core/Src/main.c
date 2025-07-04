/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c-lcd.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// --- Parameter Kontrol yang Dapat Disesuaikan ---
#define SET_POINT_TEMP 80.0f     // Setpoint suhu target dalam Celcius
#define HYSTERESIS 5.0f          // Histeresis untuk mencegah on/off terlalu cepat (pemanas menyala di bawah 75 C)
#define IGNITION_TIME_MS 3000    // Waktu pemantik menyala sebelum gas (3 detik)
#define IGNITION_OVERLAP_MS 1000 // Waktu pemantik & gas menyala bersamaan (1 detik)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;

/* Definitions for ReadTempTask */
osThreadId_t ReadTempTaskHandle;
const osThreadAttr_t ReadTempTask_attributes = {
    .name = "ReadTempTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for ControlTask */
osThreadId_t ControlTaskHandle;
const osThreadAttr_t ControlTask_attributes = {
    .name = "ControlTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for UpdateLCDTask */
osThreadId_t UpdateLCDTaskHandle;
const osThreadAttr_t UpdateLCDTask_attributes = {
    .name = "UpdateLCDTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for systemDataMutex */
osMutexId_t systemDataMutexHandle;
const osMutexAttr_t systemDataMutex_attributes = {
    .name = "systemDataMutex"};
/* USER CODE BEGIN PV */

// --- Variabel Global yang Dishare antar Task ---
float currentTemperature = -1.0f;

typedef enum
{
  STATE_IDLE,
  STATE_IGNITING,
  STATE_HEATING,
  STATE_ERROR
} SystemState_t;

SystemState_t systemState = STATE_IDLE;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
void StartReadTempTask(void *argument);
void StartControlTask(void *argument);
void StartUpdateLCDTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void InitializeSystem()
{
  lcd_init(); // sudah diinisialisasi di main
  lcd_put_cur(0, 0);
  lcd_send_string("HELLO WORLD");
  lcd_put_cur(1, 0);
  lcd_send_string("from CTECH");

  HAL_GPIO_WritePin(O_PEMANTIK_GPIO_Port, O_PEMANTIK_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(O_SELENOID_GPIO_Port, O_SELENOID_Pin, GPIO_PIN_RESET);

  osDelay(5000);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  //  lcd_init();

  //// Display Number
  //  int num = 1234;
  //  char numChar[5];
  //  sprintf(numChar, "%d", num);
  //  lcd_put_cur(0, 0);
  //  lcd_send_string (numChar);

  //  lcd_init();
  //  float flt = 12.345;
  //  char fltChar[6];
  //  sprintf(fltChar, "%.3f", flt);
  //  lcd_put_cur(0, 0);
  //  lcd_send_string (fltChar);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of systemDataMutex */
  systemDataMutexHandle = osMutexNew(&systemDataMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ReadTempTask */
  ReadTempTaskHandle = osThreadNew(StartReadTempTask, NULL, &ReadTempTask_attributes);

  /* creation of ControlTask */
  ControlTaskHandle = osThreadNew(StartControlTask, NULL, &ControlTask_attributes);

  /* creation of UpdateLCDTask */
  UpdateLCDTaskHandle = osThreadNew(StartUpdateLCDTask, NULL, &UpdateLCDTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  if (ReadTempTaskHandle == NULL)
  {
    Error_Handler();
  }
  if (ControlTaskHandle == NULL)
  {
    Error_Handler();
  }
  if (UpdateLCDTaskHandle == NULL)
  {
    Error_Handler();
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  // InitializeSystem();
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, USER_LED_Pin | O_PEMANTIK_Pin | O_SELENOID_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_BTN_Pin */
  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_HIJAU_Pin BTN_HITAM_Pin E_BTN_Pin */
  GPIO_InitStruct.Pin = BTN_HIJAU_Pin | BTN_HITAM_Pin | E_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : USER_LED_Pin O_PEMANTIK_Pin O_SELENOID_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin | O_PEMANTIK_Pin | O_SELENOID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MAX_CS_Pin */
  GPIO_InitStruct.Pin = MAX_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MAX_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartReadTempTask */
/**
 * @brief  Function implementing the ReadTempTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadTempTask */
void StartReadTempTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  uint8_t spiData[2];
  uint16_t rawTemp;
  /* Infinite loop */
  for (;;)
  {
    HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_RESET);
    osDelay(10); // Menggunakan osDelay

    if (HAL_SPI_Receive(&hspi1, spiData, 2, HAL_MAX_DELAY) == HAL_OK)
    {
      HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_SET);
      rawTemp = (spiData[0] << 8) | spiData[1];

      // Menggunakan osMutexWait untuk mengambil mutex
      if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
      {
        if (rawTemp & 0x04)
        {
          systemState = STATE_ERROR;
          currentTemperature = -1.0f;
        }
        else
        {
          currentTemperature = (rawTemp >> 3) * 0.25f;
          if (systemState == STATE_ERROR)
          {
            systemState = STATE_IDLE;
          }
        }
        osMutexRelease(systemDataMutexHandle); // Melepaskan mutex
      }
    }
    else
    {
      HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_SET);
      if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
      {
        systemState = STATE_ERROR;
        osMutexRelease(systemDataMutexHandle);
      }
    }
    osDelay(500); // Jeda task
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartControlTask */
/**
 * @brief Function implementing the ControlTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartControlTask */
void StartControlTask(void *argument)
{
  /* USER CODE BEGIN StartControlTask */
  float temp;
  SystemState_t localState;
  /* Infinite loop */
  for (;;)
  {
    if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
    {
      temp = currentTemperature;
      localState = systemState;
      osMutexRelease(systemDataMutexHandle);
    }

    switch (localState)
    {
    case STATE_IDLE:
      if (temp < (SET_POINT_TEMP - HYSTERESIS) && temp >= 0)
      {
        if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
        {
          systemState = STATE_IGNITING;
          osMutexRelease(systemDataMutexHandle);
        }
      }
      break;

    case STATE_IGNITING:
      HAL_GPIO_WritePin(O_PEMANTIK_GPIO_Port, O_PEMANTIK_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(O_SELENOID_GPIO_Port, O_SELENOID_Pin, GPIO_PIN_RESET);
      osDelay(IGNITION_TIME_MS);

      HAL_GPIO_WritePin(O_SELENOID_GPIO_Port, O_SELENOID_Pin, GPIO_PIN_SET);
      osDelay(IGNITION_OVERLAP_MS);

      HAL_GPIO_WritePin(O_PEMANTIK_GPIO_Port, O_PEMANTIK_Pin, GPIO_PIN_RESET);

      if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
      {
        systemState = STATE_HEATING;
        osMutexRelease(systemDataMutexHandle);
      }
      break;

    case STATE_HEATING:
      if (temp >= SET_POINT_TEMP)
      {
        HAL_GPIO_WritePin(O_SELENOID_GPIO_Port, O_SELENOID_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(O_PEMANTIK_GPIO_Port, O_PEMANTIK_Pin, GPIO_PIN_RESET);
        if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
        {
          systemState = STATE_IDLE;
          osMutexRelease(systemDataMutexHandle);
        }
      }
      break;

    case STATE_ERROR:
      HAL_GPIO_WritePin(O_SELENOID_GPIO_Port, O_SELENOID_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(O_PEMANTIK_GPIO_Port, O_PEMANTIK_Pin, GPIO_PIN_RESET);
      break;
    }
    osDelay(200);
  }
  /* USER CODE END StartControlTask */
}

/* USER CODE BEGIN Header_StartUpdateLCDTask */
/**
 * @brief Function implementing the UpdateLCDTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartUpdateLCDTask */
void StartUpdateLCDTask(void *argument)
{
  /* USER CODE BEGIN StartUpdateLCDTask */
  char tempStr[17];
  char statusStr[17];
  float temp;
  SystemState_t localState;

  InitializeSystem();

  /* Infinite loop */
  for (;;)
  {
    if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
    {
      temp = currentTemperature;
      localState = systemState;
      osMutexRelease(systemDataMutexHandle);
    }

    if (temp < 0)
    {
      sprintf(tempStr, "Suhu: --- C  ");
    }
    else
    {
      sprintf(tempStr, "Suhu: %.1f C", temp);
    }
    sprintf(statusStr, "Set:%.0f  Sts:", SET_POINT_TEMP);

    switch (localState)
    {
    case STATE_IDLE:
      strcat(statusStr, "IDLE ");
      break;
    case STATE_IGNITING:
      strcat(statusStr, "IGNIT");
      break;
    case STATE_HEATING:
      strcat(statusStr, "HEAT ");
      break;
    case STATE_ERROR:
      sprintf(statusStr, "!!SENSOR ERROR!!");
      break;
    }

    lcd_put_cur(0, 0);
    lcd_send_string(tempStr);
    lcd_put_cur(0, 1);
    lcd_send_string(statusStr);

    osDelay(1000);
  }
  /* USER CODE END StartUpdateLCDTask */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM4 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    // Stay in this loop to indicate an error
    // You can also toggle an LED or send a message to the LCD here
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    osDelay(500); // Delay to make the error indication visible
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
