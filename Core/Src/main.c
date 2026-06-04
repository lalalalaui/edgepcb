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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>

#include "app_ai.h"
#include "test_vectors.h"
#include "thresholds.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/LVGL/lvgl_port.h"
#include "./BSP/LVGL/pcb_ai_ui.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/TOUCH/touch.h"

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

/* USER CODE BEGIN PV */
static uint32_t current_sample_index = 0;
static int g_ai_ready = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void) __attribute__((unused));
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
_Static_assert(TEST_VECTOR_COUNT > 0, "TEST_VECTOR_COUNT must be > 0");
_Static_assert(TEST_INPUT_SIZE == 27648, "TEST_INPUT_SIZE must be 27648");

static const char *app_score_result(float score)
{
  if (score < TINY_AE_THRESHOLD_LOW) {
    return "NORMAL";
  }
  if (score < TINY_AE_THRESHOLD_HIGH) {
    return "SUSPECT";
  }
  return "ANOMALY";
}

static int app_score_result_code(float score)
{
  if (score < TINY_AE_THRESHOLD_LOW) {
    return 0;
  }
  if (score < TINY_AE_THRESHOLD_HIGH) {
    return 1;
  }
  return 2;
}

static void run_current_sample(void)
{
  float score = 0.0f;
  uint32_t latency_ms = 0;
  const uint32_t sample_index = current_sample_index;
  const uint8_t expected_label = g_test_label[sample_index];
  const char *result = NULL;
  const float *output = NULL;
  int result_code = 0;

  if (!g_ai_ready) {
    printf("AI not ready: %s\r\n", app_ai_last_error());
    pcb_ai_ui_set_status("AI not ready");
    return;
  }

  if (app_ai_run(g_test_inputs[sample_index], &score, &latency_ms) != 0) {
    printf("sample=%lu AI run failed: %s\r\n",
           (unsigned long)sample_index,
           app_ai_last_error());
    pcb_ai_ui_set_status("AI run failed");
    return;
  }

  result = app_score_result(score);
  result_code = app_score_result_code(score);
  output = app_ai_get_output();

#if defined(TEST_VECTOR_HAS_EXPECTED_SCORE)
  const double pc_ref = (double)g_test_expected_score[sample_index];
  const double abs_error = fabs((double)score - pc_ref);
  const char *compare = (abs_error < 1.0e-6) ? "MATCH" : "CHECK_PREPROCESS_OR_BUFFER";

  printf("sample=%lu label=%u score=%.9g pc_ref=%.9g abs_error=%.12f threshold_low=%.9g threshold_high=%.9g result=%s latency=%lums compare=%s\r\n",
         (unsigned long)sample_index,
         (unsigned)expected_label,
         (double)score,
         pc_ref,
         abs_error,
         (double)TINY_AE_THRESHOLD_LOW,
         (double)TINY_AE_THRESHOLD_HIGH,
         result,
         (unsigned long)latency_ms,
         compare);
  pcb_ai_ui_update_result(score,
                          TINY_AE_THRESHOLD_LOW,
                          TINY_AE_THRESHOLD_HIGH,
                          result_code,
                          latency_ms,
                          (int)sample_index,
                          TEST_VECTOR_COUNT,
                          expected_label,
                          compare);
#else
  const char *compare = "NO_PC_REF";
  printf("sample=%lu label=%u score=%.9g pc_ref=NA abs_error=NA threshold_low=%.9g threshold_high=%.9g result=%s latency=%lums compare=NO_PC_REF\r\n",
         (unsigned long)sample_index,
         (unsigned)expected_label,
         (double)score,
         (double)TINY_AE_THRESHOLD_LOW,
         (double)TINY_AE_THRESHOLD_HIGH,
         result,
         (unsigned long)latency_ms);
  pcb_ai_ui_update_result(score,
                          TINY_AE_THRESHOLD_LOW,
                          TINY_AE_THRESHOLD_HIGH,
                          result_code,
                          latency_ms,
                          (int)sample_index,
                          TEST_VECTOR_COUNT,
                          expected_label,
                          compare);
#endif
  pcb_ai_ui_show_input_patch(g_test_inputs[sample_index]);
  pcb_ai_ui_show_heatmap(g_test_inputs[sample_index], output);
  pcb_ai_ui_set_status("Last run ok");
}

static void next_sample(void)
{
  current_sample_index++;
  if (current_sample_index >= TEST_VECTOR_COUNT) {
    current_sample_index = 0;
  }
  run_current_sample();
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

  sys_cache_enable();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  delay_init(400);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  usart_init(115200);
  printf("UART ok\r\n");

  sdram_init();
  printf("SDRAM init ok\r\n");

  lcd_init();
  printf("LCD init ok\r\n");

  tp_dev.init();
  printf("Touch init ok\r\n");

  lvgl_port_init();
  pcb_ai_ui_create();
  pcb_ai_ui_set_run_callback(run_current_sample);
  pcb_ai_ui_set_next_callback(next_sample);
  pcb_ai_ui_set_status("AI init");

  if (app_ai_init() == 0) {
    g_ai_ready = 1;
    printf("AI init ok\r\n");
    pcb_ai_ui_set_status("AI init ok");
    run_current_sample();
  } else {
    g_ai_ready = 0;
    printf("AI init failed: %s\r\n", app_ai_last_error());
    pcb_ai_ui_set_status("AI init failed");
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    lv_timer_handler();
    HAL_Delay(5);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  uint8_t ret = 0;

  ret = sys_stm32_clock_init(160, 5, 2, 4);
  if (ret != 0U)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

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
