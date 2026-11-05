/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "hts221.h"
#include "lps25hb.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MEASUREMENT_PERIOD_MS		100U
#define SENSOR_RETRY_PERIOD_MS		1000U
#define REFERENCE_SAMPLES			16U
#define REFERENCE_SKIPPED_SAMPLES	10U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static float reference_pressure = 0.0f;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void sensors_init(void);
static float measure_reference_pressure(void);
static void format_fixed(char *buffer, uint8_t size, float value, uint8_t decimals);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  hts221_register_callbacks(i2c_master_read, i2c_master_write);
  lps25hb_register_callbacks(i2c_master_read, i2c_master_write);

  sensors_init();

  reference_pressure = measure_reference_pressure();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  float temperature, humidity, pressure, altitude;
	  char temperature_str[12], humidity_str[12], pressure_str[12], altitude_str[12];
	  char message[64];

	  if(hts221_get_temperature(&temperature) && hts221_get_humidity(&humidity) &&
		 lps25hb_get_pressure(&pressure))
	  {
		  altitude = lps25hb_calculate_altitude(pressure, reference_pressure);

		  format_fixed(temperature_str, sizeof(temperature_str), temperature, 1);
		  format_fixed(humidity_str, sizeof(humidity_str), humidity, 0);
		  format_fixed(pressure_str, sizeof(pressure_str), pressure, 2);
		  format_fixed(altitude_str, sizeof(altitude_str), altitude, 2);

		  snprintf(message, sizeof(message), "%s,%s,%s,%s\r\n",
				   temperature_str, humidity_str, pressure_str, altitude_str);
		  usart2_send_string(message);
	  }
	  else
	  {
		  usart2_send_string("Sensor read error\r\n");
		  sensors_init();
	  }

	  LL_mDelay(MEASUREMENT_PERIOD_MS);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SetSystemCoreClock(8000000);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
}

/* USER CODE BEGIN 4 */

static void sensors_init(void)
{
	uint8_t humidity_sensor_ok, pressure_sensor_ok;

	while(1)
	{
		humidity_sensor_ok = hts221_init();
		pressure_sensor_ok = lps25hb_init();

		if(humidity_sensor_ok && pressure_sensor_ok) break;

		if(!humidity_sensor_ok) usart2_send_string("Humidity sensor not found\r\n");
		if(!pressure_sensor_ok) usart2_send_string("Pressure sensor not found\r\n");

		LL_mDelay(SENSOR_RETRY_PERIOD_MS);
	}
}

static float measure_reference_pressure(void)
{
	float sum = 0.0f;
	float pressure;
	uint8_t samples = 0;
	uint8_t skipped = 0;
	uint16_t timeout = 0;

	while((samples < REFERENCE_SAMPLES) && (timeout < 5000U))
	{
		if(lps25hb_is_pressure_ready() && lps25hb_get_pressure(&pressure))
		{
			if(skipped < REFERENCE_SKIPPED_SAMPLES)
			{
				skipped++;
			}
			else
			{
				sum += pressure;
				samples++;
			}
		}
		LL_mDelay(1);
		timeout++;
	}

	if(samples == 0U)
	{
		return lps25hb_get_pressure(&pressure) ? pressure : 1013.25f;
	}

	return sum / (float)samples;
}

static void format_fixed(char *buffer, uint8_t size, float value, uint8_t decimals)
{
	int32_t scale = 1;
	int32_t scaled;
	uint8_t i;

	for(i = 0; i < decimals; i++) scale *= 10;

	scaled = (int32_t)(value * (float)scale + ((value >= 0.0f) ? 0.5f : -0.5f));

	const char *sign = (scaled < 0) ? "-" : "";
	if(scaled < 0) scaled = -scaled;

	if(decimals == 0U)
	{
		snprintf(buffer, size, "%s%ld", sign, (long)scaled);
	}
	else if(decimals == 1U)
	{
		snprintf(buffer, size, "%s%ld.%01ld", sign, (long)(scaled / scale), (long)(scaled % scale));
	}
	else
	{
		snprintf(buffer, size, "%s%ld.%02ld", sign, (long)(scaled / scale), (long)(scaled % scale));
	}
}

/* USER CODE END 4 */

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
