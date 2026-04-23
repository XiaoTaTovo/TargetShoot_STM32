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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "function.h"
#include "key.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "PID.h"
#include "serial.h"
#include "servo.h"
#include "ServoPID.h"
#include "task.h"
#include "alarm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
Alarm_Controller_t MyAlarm;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t Vision_RxBuffer[50];//接收树莓派数据的数组
uint8_t Vision_RxFlag = 0;//接收中断标志位，接收到置1

// 🌟 新增：蓝牙全局变量
uint8_t BT_RxFlag = 0;
uint8_t BT_RxBuffer[50] = {0};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim9) {
    Key_Tick();

  }

}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim9);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_UART_Receive_DMA(&huart1, (uint8_t *)Vision_RxBuffer, 50);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  HAL_UART_Receive_DMA(&huart2, (uint8_t *)BT_RxBuffer, 50);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
  OLED_Init();
  Servo_Init();
  Alarm_Init(&MyAlarm);
  Task_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    uint8_t Key_Num = Key_GetNum();

    // ==========================================
    // 🟢 按键 1 (PA4)：纯粹的模式切换
    // ==========================================
    if (Key_Num == 1) {
      System_Mode++;
      if (System_Mode > 5) System_Mode = 1;
      Serial_Printf(&huart1, "MODE:%d\n", System_Mode); // 通知上位机
      Alarm_Start_Beep(&MyAlarm, 1);
    }

    // ==========================================
    // 🟡 按键 2 (PA5)：复位归中 + 唤醒启动 (Start)
    // ==========================================
    else if (Key_Num == 2) {
      // 🌟 每次按下，状态翻转 (0变1，1变0)
      System_Run = !System_Run;

      if (System_Run == 0) {
        // 【待机模式】：洗脑 + 回中 + 保持力量！(绝不松手)
        PID_Init(&PID_Yaw);
        PID_Init(&PID_Pitch);
        Servo_SetYaw(0.0f);
        Servo_SetPitch(0.0f);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

        Alarm_Start_Beep(&MyAlarm, 2); // 滴滴两声，提示进入“归中锁死状态”
      } else {
        // 【追踪模式】：释放猛兽！
        Alarm_Start_Beep(&MyAlarm, 1); // 滴一声长音，提示“开始追踪！”
      }
    }

    // ==========================================
    // 🔴 按键 3 (PB6)：紧急断电 (Stop)
    // ==========================================
    else if (Key_Num == 3) {
      // 🌟 1. 必须先断电！防止舵机收到归零信号乱扭！
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);

      // 2. 断电后再慢慢清空大脑记忆
      PID_Init(&PID_Yaw);
      PID_Init(&PID_Pitch);

      Alarm_Start_Beep(&MyAlarm, 3);
      Alarm_Start_Blink(&MyAlarm, 1);
    }
    // 核心：一刻不停地检查串口缓冲区是否有新数据 (非阻塞)
    Vision_Data_Proceed();
    // 核心：进入调度中心
    Task_Scheduler();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
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

/* USER CODE BEGIN 4 */

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
