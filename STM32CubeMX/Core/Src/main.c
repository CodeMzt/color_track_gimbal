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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "control.h"
#include "oled.h"
#include "key.h"
#include "bus_servo.h" // 总线舵机驱动
#include "delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 二维云台：上层舵机 ID=000；下层舵机 ID=001 */
static const uint8_t SERVO_UPPER_ID = 0;
static const uint8_t SERVO_LOWER_ID = 1;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
key_s key;
volatile uint8_t kv = 0; // 全局按键值，由定时中断周期性采样更新
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int parse_packet(const char *s, int *out_a, int *out_b);
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
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  key.GPIOx = KEY_GPIO_Port;
  key.pin = KEY_Pin;
  key_init(&key);

  // 初始化 OLED 并显示标签（16x4 屏，x=0..15, y=0..3）
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(0,3,"Gimbal Test",11);
  HAL_TIM_Base_Start_IT(&htim1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // 先将两层舵机移动到中值（1500），仅执行一次
  {
  char cmd[32] = {0};
  if (bus_servo_fmt_move(cmd, sizeof(cmd), SERVO_UPPER_ID, 1500, 1000) >= 0) {
    (void)bus_servo_send(&huart1, cmd, 50);
  }
    if (bus_servo_fmt_move(cmd, sizeof(cmd), SERVO_LOWER_ID, 1500, 1000) >= 0) {
      (void)bus_servo_send(&huart1, cmd, 50);
    }
  }
  while (1)
  {
    char cmd[16]={0};
    char buf[16]="";
    int received_x=0, received_y=0;
    static int x=0,y=0;
    HAL_UART_Receive(&huart2, buf,sizeof(buf), 10);
    parse_packet(buf, &received_x, &received_y);
    // OLED_ShowSignNum(0,0,received_x,3);
    // OLED_ShowSignNum(0,1,received_y,3);
    control_set_target(x*0.05+received_x*0.95,y*0.05+received_y*0.95);
    x=received_x;
    y=received_y;
    control(&huart1);
    /* 中值调整 */
    // if (bus_servo_fmt_set_sck_base(cmd,sizeof(cmd),SERVO_UPPER_ID)>=0) {
    //   if (bus_servo_send(&huart1,cmd,50)!=HAL_OK) {
    //     // 发送失败处理（可选）
    //     OLED_ShowString(0,2,"NO",2);
    //   }
    //   else OLED_ShowString(0,2,"ok",2);
    // }
    // if (bus_servo_fmt_set_sck_base(cmd,sizeof(cmd),SERVO_LOWER_ID)>=0) {
    //    if (bus_servo_send(&huart1,cmd,50)!=HAL_OK) {
    //     // 发送失败处理（可选）
    //     OLED_ShowString(0,3,"NO",2);
    //   }
    //   else OLED_ShowString(0,3,"ok",2);
    // }

    // --- 控制测试：根据 kv 设定/反向目标并执行控制 ---
    // {
    //   static int8_t x_sign = +1; // 当前 x 方向符号
    //   static int8_t y_sign = +1; // 当前 y 方向符号
    //   uint8_t k = kv; // 读取快照，避免中途变化
    //   if (k == 4) {
    //     control_set_target(10.0f * x_sign, 0.0f);
    //   } else if (k == 1) {
    //     control_set_target(0.0f, 10.0f * y_sign);
    //   } else if (k == 2) {
    //     // 反向：翻转已设定目标的符号。如果当前某轴目标为0，则保持0
    //     x_sign = -x_sign;
    //     y_sign = -y_sign;
    //   }
    //   // 执行一次控制输出（根据 PID 目标更新两个舵机）
    //   (void)control(&huart1);
    // }

    // 节流，避免过快刷新（不使用自定义Delay以免影响SysTick）
    //HAL_Delay(50);
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

/* USER CODE BEGIN 4 */
/**
  * @brief  Timer period elapsed callback in non-blocking mode
  * @param  htim: Pointer to a TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  static uint32_t time_cnt=0;
  if (htim->Instance == TIM1) {
    /* USER CODE BEGIN TIM1_PERIOD_ELAPSED */
    // time_cnt++;
    // if (time_cnt>=20) {
    //   time_cnt=0;
    //   key_scan(&key); // 保留按键去抖
    // }
    // // 每100个计数采样一次按键值
    // static uint32_t sample_cnt = 0;
    // sample_cnt++;
    // if (sample_cnt >= 100) {
    //   sample_cnt = 0;
    //   kv = key_get_state(&key);
    // }
    /* USER CODE END TIM1_PERIOD_ELAPSED */
  }
}
int parse_packet(const char *s, int *out_a, int *out_b) {
  // Skip leading whitespace
  while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
  if (*s != '$') return -1;
  s++; // after leading '$'
  // Parse first integer
  char *end = NULL;
  long a = strtol(s, &end, 10);
  if (end == s) return -1; // no digits
  s = end;
  // Skip spaces between numbers
  while (*s == ' ' || *s == '\t') s++;
  // Parse second integer
  long b = strtol(s, &end, 10);
  if (end == s) return -1; // no second digits
  s = end;
  // Skip trailing spaces
  while (*s == ' ' || *s == '\t') s++;
  if (*s != '$') return -1; // must end with '$'
  // Range clamp to int if needed
  *out_a = (int)a;
  *out_b = (int)b;
  return 0;
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
