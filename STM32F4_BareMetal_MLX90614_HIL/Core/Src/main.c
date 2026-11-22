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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile bool tim2_Flag=false;
char tx_buffer[64];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void GPIO_Init(void);
void TIM2_Init(void);
void I2C1_Init(void);
void USART2_Init(void);
uint16_t I2C_Read_Byte(uint8_t dev_addr,uint8_t reg_addr);
void UART_SendString(const char *str);
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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  GPIO_Init();
  TIM2_Init();
  I2C1_Init();
  USART2_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (tim2_Flag){
		  tim2_Flag=false;
		  uint16_t raw_obj1 = I2C_Read_Byte(MLX90614_ADDR,MLX90614_REG_1);
		  uint16_t raw_obj2 = I2C_Read_Byte(MLX90614_ADDR,MLX90614_REG_2);

		  // Convert raw values to Celsius
		  float temp1_C = (raw_obj1 * 0.02f) - 273.15f;
		  float temp2_C = (raw_obj2 * 0.02f) - 273.15f;

		  // Format Object 1 to 1 decimal place
		  int t1_int = (int)temp1_C;
		  int t1_dec = (int)((temp1_C - t1_int) * 10.0f);
		  if (t1_dec < 0) t1_dec = -t1_dec;

		  // Format Object 2 to 1 decimal place
		  int t2_int = (int)temp2_C;
		  int t2_dec = (int)((temp2_C - t2_int) * 10.0f);
		  if (t2_dec < 0) t2_dec = -t2_dec;
		  // Transmit over USART2
		  if (Temp1 > 36.0f) {
		      GPIOC->BSRR = (1 << (13 + 16));   // PC13 LOW (LED ON on BlackPill)
		      sprintf(tx_buffer, "[TEMP1] %.1f C | [TEMP2] %.1f C | STATUS: OVER-TEMP ALERT!\r\n", Temp1, Temp2);
		  } else {
		      GPIOC->BSRR = (1 << 13);          // PC13 HIGH (LED OFF)
		      sprintf(tx_buffer, "[TEMP1] %.1f C | [TEMP2] %.1f C | STATUS: NORMAL\r\n", Temp1, Temp2);
		  }

	  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
 void GPIO_Init(void){
	 RCC->AHB1ENR |= (1<<0) | (1<<1);

	 GPIOA->MODER&= ~((3<< 2*2) | (3<< 2*3));
	 GPIOA->MODER |= ((2<< 2*2) | (2<< 2*3));

	 GPIOA->AFR[0]&= ~((0xF << 4*2) | (0xF <<4*3));
	 GPIOA->AFR[0]|= ((7<< 4*2) | (7 << 4*3));

	 GPIOB->MODER&= ~((3<< 2*6) | (3<< 2*7));
	 GPIOB->MODER |= ((2<< 2*6) | (2<< 2*7));

	 GPIOB->OTYPER |= (1<<6) | (1<<7);

	 GPIOB->OSPEEDR |= (3<<2*6) | (3<<2*7);
	 GPIOB->PUPDR&= ~((3<< 2*6) | (3<< 2*7));
	 GPIOB->PUPDR|= ((1<< 2*6) | (1<< 2*7));

	 GPIOB->AFR[0]&= ~((0xF << 4*6) | (0xF <<4*7));
	 GPIOB->AFR[0]|= ((4<< 4*6) | (4 << 4*7));
 }

 void I2C1_Init(void){
	 RCC->APB1ENR |= (1<<21);

	 I2C1->CR1 |= (1<<15);
	 I2C1->CR1 &= ~(1<<15);

	 I2C1->CR2 &= ~(0x3F);
	 I2C1->CR2 |=42;

	 I2C1->CCR &= ~(1<<15);
	 I2C1->CCR =210;

	 I2C1->TRISE=43;

	 I2C1->CR1 |= (1<<10);
	 I2C1->CR1 |= (1<<0);
 }

 void TIM2_Init(void){
	 RCC->APB1ENR|=(1<<0);

	 TIM2->PSC=15999;
	 TIM2->ARR=999;

	 TIM2->DIER |= (1<<0);
	 NVIC_SetPriority(TIM2_IRQn,2);
	 NVIC_EnableIRQ(TIM2_IRQn);

	 TIM2->CR1 |= (1<<0);
 }

 void USART2_Init(void){
	RCC->APB1ENR |= (1<<17);

	USART2->BRR= BRR_VALUE;

	USART2->CR1 |= (1<<3);
	USART2->CR1 |= (1<<13);
 }

 void UART_SendString(const char *str){
	 while(*str){
		 while(!(USART2->SR & (1<<7)));
		 USART2->DR = *str++;
	 }
 }

uint16_t I2C_Read_Byte(uint8_t dev_addr,uint8_t reg_addr){

	while (I2C1->SR2 & (1<<1));

	I2C1->CR1|= (1<<8);
	while(!(I2C1->SR1 &(1<<0)));

	I2C1->DR=(dev_addr<<1) | 0;
	while(!(I2C1->SR1 & (1<<1)));
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    while(!(I2C1->SR1 & (1<<7)));
    I2C1->DR=reg_addr;
    while(!(I2C1->SR1 & (1<<7)));

    I2C1->CR1 |= (1<<8);
    while(!(I2C1->SR1 &(1<<0)));

    I2C1->CR1 |=(1<<10);
    I2C1->DR=(dev_addr<<1) | 1;
	while(!(I2C1->SR1 & (1<<1)));
	(void)I2C1->SR1;
	(void)I2C1->SR2;

	while(!(I2C1->SR1 & (1<<6)));
	uint8_t low=I2C1->DR;

	while(!(I2C1->SR1 & (1<<6)));
	uint8_t high=I2C1->DR;

	I2C1->CR1 &= ~(1<<10);
	I2C1->CR1 |= (1<<9);

	while(!(I2C1->SR1 & (1<<6)));
	uint8_t pec =I2C1->DR;
	(void)pec;

	I2C1->CR1 |= (1<<10);

	return ((uint16_t) high << 8) | low;
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
