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
#include "test_vectors_compact.h"
#include "thresholds.h"
#include "tiny_classifier_test_vectors_compact.h"
#include "app_tiny_classifier.h"
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
static uint32_t current_classifier_sample_index = 0;
static int g_ai_ready = 0;
static int g_classifier_ready = 0;
static int g_current_mode = PCB_AI_UI_MODE_ANOMALY;

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define MAIN_SDRAM_AI_ATTR __attribute__((section(".sdram_ai"), used, aligned(32)))
#else
#define MAIN_SDRAM_AI_ATTR
#endif

static float s_decoded_input[TEST_INPUT_SIZE] MAIN_SDRAM_AI_ATTR;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
_Static_assert(TEST_VECTOR_COUNT > 0, "TEST_VECTOR_COUNT must be > 0");
_Static_assert(TEST_INPUT_SIZE == 27648, "TEST_INPUT_SIZE must be 27648");
_Static_assert(TINY_CLASSIFIER_TEST_VECTOR_COUNT > 0, "TINY_CLASSIFIER_TEST_VECTOR_COUNT must be > 0");
_Static_assert(TINY_CLASSIFIER_INPUT_SIZE == 27648, "TINY_CLASSIFIER_INPUT_SIZE must be 27648");
_Static_assert(TINY_CLASSIFIER_NUM_CLASSES == 6, "TINY_CLASSIFIER_NUM_CLASSES must be 6");

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

static int decode_anomaly_input(uint32_t sample_index, float *dst)
{
  uint32_t out_index = 0;

  if ((sample_index >= TEST_VECTOR_COUNT) || (dst == NULL)) {
    return -1;
  }

  for (uint32_t i = 0; i < g_test_input_rle_count[sample_index]; i++) {
    const test_vector_rle_t item = g_test_input_rle[sample_index][i];
    for (uint32_t j = 0; j < item.count; j++) {
      if (out_index >= TEST_INPUT_SIZE) {
        return -1;
      }
      dst[out_index++] = item.value;
    }
  }

  return (out_index == TEST_INPUT_SIZE) ? 0 : -1;
}

static int decode_classifier_input(uint32_t sample_index, float *dst)
{
  uint32_t out_index = 0;

  if ((sample_index >= TINY_CLASSIFIER_TEST_VECTOR_COUNT) || (dst == NULL)) {
    return -1;
  }

  for (uint32_t i = 0; i < g_tiny_classifier_input_rle_count[sample_index]; i++) {
    const tiny_classifier_vector_rle_t item = g_tiny_classifier_input_rle[sample_index][i];
    for (uint32_t j = 0; j < item.count; j++) {
      if (out_index >= TINY_CLASSIFIER_INPUT_SIZE) {
        return -1;
      }
      dst[out_index++] = item.value;
    }
  }

  return (out_index == TINY_CLASSIFIER_INPUT_SIZE) ? 0 : -1;
}

static void run_current_anomaly_sample(void)
{
  float score = 0.0f;
  uint32_t latency_ms = 0;
  const uint32_t sample_index = current_sample_index;
  const uint8_t expected_label = g_test_label[sample_index];
  const char *result = NULL;
  const float *output = NULL;
  int result_code = 0;

  if (!g_ai_ready) {
    printf("mode=ANOMALY AI not ready: %s\r\n", app_ai_last_error());
    pcb_ai_ui_set_status("AI not ready");
    return;
  }

  if (decode_anomaly_input(sample_index, s_decoded_input) != 0) {
    printf("mode=ANOMALY sample=%lu decode failed\r\n", (unsigned long)sample_index);
    pcb_ai_ui_set_status("Anomaly decode failed");
    return;
  }

  if (app_ai_run(s_decoded_input, &score, &latency_ms) != 0) {
    printf("mode=ANOMALY sample=%lu AI run failed: %s\r\n",
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

  printf("mode=ANOMALY sample=%lu label=%u score=%.9g pc_ref=%.9g abs_error=%.12f threshold_low=%.9g threshold_high=%.9g result=%s latency=%lums compare=%s\r\n",
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
  printf("mode=ANOMALY sample=%lu label=%u score=%.9g pc_ref=NA abs_error=NA threshold_low=%.9g threshold_high=%.9g result=%s latency=%lums compare=NO_PC_REF\r\n",
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
  pcb_ai_ui_show_input_patch(s_decoded_input);
  pcb_ai_ui_show_heatmap(s_decoded_input, output);
  pcb_ai_ui_set_status("Last run ok");
}

static void classifier_top3(const tiny_classifier_result_t *result, int top3[3])
{
  int indices[TINY_CLASSIFIER_NUM_CLASSES];

  for (int i = 0; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
    indices[i] = i;
  }

  for (int i = 0; i < TINY_CLASSIFIER_NUM_CLASSES - 1; i++) {
    for (int j = i + 1; j < TINY_CLASSIFIER_NUM_CLASSES; j++) {
      if (result->probs[indices[j]] > result->probs[indices[i]]) {
        const int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
      }
    }
  }

  top3[0] = indices[0];
  top3[1] = indices[1];
  top3[2] = indices[2];
}

static void run_current_classifier_sample(void)
{
  tiny_classifier_result_t result;
  uint32_t latency_ms = 0;
  const uint32_t sample_index = current_classifier_sample_index;
  const uint8_t expected_label = g_tiny_classifier_label[sample_index];
  int top3[3] = {0, 1, 2};
  char top3_line0[32];
  char top3_line1[32];
  char top3_line2[32];

  if (!g_classifier_ready) {
    printf("mode=CLASSIFIER classifier not ready: %s\r\n", app_tiny_classifier_last_error());
    pcb_ai_ui_set_status("Classifier not ready");
    return;
  }

  if (decode_classifier_input(sample_index, s_decoded_input) != 0) {
    printf("mode=CLASSIFIER sample=%lu decode failed\r\n", (unsigned long)sample_index);
    pcb_ai_ui_set_status("Classifier decode failed");
    return;
  }

  if (app_tiny_classifier_run(s_decoded_input, &result, &latency_ms) != 0) {
    printf("mode=CLASSIFIER sample=%lu run failed: %s\r\n",
           (unsigned long)sample_index,
           app_tiny_classifier_last_error());
    pcb_ai_ui_set_status("Classifier run failed");
    return;
  }

  classifier_top3(&result, top3);
  snprintf(top3_line0, sizeof(top3_line0), "%s %.3f",
           app_tiny_classifier_class_name(top3[0]), (double)result.probs[top3[0]]);
  snprintf(top3_line1, sizeof(top3_line1), "%s %.3f",
           app_tiny_classifier_class_name(top3[1]), (double)result.probs[top3[1]]);
  snprintf(top3_line2, sizeof(top3_line2), "%s %.3f",
           app_tiny_classifier_class_name(top3[2]), (double)result.probs[top3[2]]);

#if defined(TINY_CLASSIFIER_TEST_HAS_EXPECTED_LOGITS)
  double max_abs_logit_error = 0.0;
  for (int i = 0; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
    const double err = fabs((double)result.logits[i] -
                            (double)g_tiny_classifier_expected_logits[sample_index][i]);
    if (err > max_abs_logit_error) {
      max_abs_logit_error = err;
    }
  }
  const uint8_t expected_top1 = g_tiny_classifier_expected_top1[sample_index];
  const char *compare = ((result.top1_index == expected_top1) && (max_abs_logit_error < 1.0e-5))
                        ? "MATCH" : "CHECK_CLASSIFIER";
  printf("mode=CLASSIFIER sample=%lu label=%u pred=%s top1=%d prob=%.6f expected_top1=%u max_abs_logit_error=%.9g latency=%lums compare=%s top3=[%s; %s; %s]\r\n",
         (unsigned long)sample_index,
         (unsigned)expected_label,
         app_tiny_classifier_class_name(result.top1_index),
         result.top1_index,
         (double)result.top1_prob,
         (unsigned)expected_top1,
         max_abs_logit_error,
         (unsigned long)latency_ms,
         compare,
         top3_line0,
         top3_line1,
         top3_line2);
#else
  printf("mode=CLASSIFIER sample=%lu label=%u pred=%s top1=%d prob=%.6f latency=%lums top3=[%s; %s; %s]\r\n",
         (unsigned long)sample_index,
         (unsigned)expected_label,
         app_tiny_classifier_class_name(result.top1_index),
         result.top1_index,
         (double)result.top1_prob,
         (unsigned long)latency_ms,
         top3_line0,
         top3_line1,
         top3_line2);
#endif

  pcb_ai_ui_show_input_patch(s_decoded_input);
  pcb_ai_ui_clear_heatmap();
  pcb_ai_ui_update_classifier_result((int)sample_index,
                                     TINY_CLASSIFIER_TEST_VECTOR_COUNT,
                                     expected_label,
                                     app_tiny_classifier_class_name(result.top1_index),
                                     result.top1_prob,
                                     top3_line0,
                                     top3_line1,
                                     top3_line2,
                                     latency_ms);
  pcb_ai_ui_set_status("Last run ok");
}

static void run_current_sample(void)
{
  if (g_current_mode == PCB_AI_UI_MODE_CLASSIFIER) {
    run_current_classifier_sample();
  } else {
    run_current_anomaly_sample();
  }
}

static void preview_current_sample(void)
{
  if (g_current_mode == PCB_AI_UI_MODE_CLASSIFIER) {
    const uint32_t sample_index = current_classifier_sample_index;
    const uint8_t expected_label = g_tiny_classifier_label[sample_index];

    if (decode_classifier_input(sample_index, s_decoded_input) != 0) {
      printf("mode=CLASSIFIER sample=%lu preview decode failed\r\n", (unsigned long)sample_index);
      pcb_ai_ui_set_status("Classifier decode failed");
      return;
    }

    pcb_ai_ui_show_input_patch(s_decoded_input);
    pcb_ai_ui_clear_heatmap();
    pcb_ai_ui_update_sample_info((int)sample_index,
                                 TINY_CLASSIFIER_TEST_VECTOR_COUNT,
                                 expected_label);
    pcb_ai_ui_set_status("Sample selected");
    printf("mode=CLASSIFIER sample=%lu label=%u selected\r\n",
           (unsigned long)sample_index,
           (unsigned)expected_label);
  } else {
    const uint32_t sample_index = current_sample_index;
    const uint8_t expected_label = g_test_label[sample_index];

    if (decode_anomaly_input(sample_index, s_decoded_input) != 0) {
      printf("mode=ANOMALY sample=%lu preview decode failed\r\n", (unsigned long)sample_index);
      pcb_ai_ui_set_status("Anomaly decode failed");
      return;
    }

    pcb_ai_ui_show_input_patch(s_decoded_input);
    pcb_ai_ui_clear_heatmap();
    pcb_ai_ui_update_sample_info((int)sample_index,
                                 TEST_VECTOR_COUNT,
                                 expected_label);
    pcb_ai_ui_set_status("Sample selected");
    printf("mode=ANOMALY sample=%lu label=%u selected\r\n",
           (unsigned long)sample_index,
           (unsigned)expected_label);
  }
}

static void next_sample(void)
{
  if (g_current_mode == PCB_AI_UI_MODE_CLASSIFIER) {
    current_classifier_sample_index++;
    if (current_classifier_sample_index >= TINY_CLASSIFIER_TEST_VECTOR_COUNT) {
      current_classifier_sample_index = 0;
    }
  } else {
    current_sample_index++;
    if (current_sample_index >= TEST_VECTOR_COUNT) {
      current_sample_index = 0;
    }
  }
  preview_current_sample();
}

static void select_anomaly_mode(void)
{
  g_current_mode = PCB_AI_UI_MODE_ANOMALY;
  printf("current mode=ANOMALY\r\n");
  pcb_ai_ui_set_mode(PCB_AI_UI_MODE_ANOMALY);
  preview_current_sample();
}

static void select_classifier_mode(void)
{
  g_current_mode = PCB_AI_UI_MODE_CLASSIFIER;
  printf("current mode=CLASSIFIER\r\n");
  pcb_ai_ui_set_mode(PCB_AI_UI_MODE_CLASSIFIER);
  preview_current_sample();
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */
  sys_cache_enable();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  if (sys_stm32_clock_init(160, 5, 2, 4) != 0) {
    Error_Handler();
  }

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
  printf("LVGL port init ok\r\n");
  pcb_ai_ui_create();
  printf("PCB UI create ok\r\n");
  pcb_ai_ui_set_run_callback(run_current_sample);
  pcb_ai_ui_set_next_callback(next_sample);
  pcb_ai_ui_set_anomaly_callback(select_anomaly_mode);
  pcb_ai_ui_set_classifier_callback(select_classifier_mode);
  pcb_ai_ui_set_mode(PCB_AI_UI_MODE_ANOMALY);
  pcb_ai_ui_set_status("AI init");
  lv_timer_handler();
  HAL_Delay(20);

  printf("AI init start\r\n");
  if (app_ai_init() == 0) {
    g_ai_ready = 1;
    printf("AI init ok\r\n");
    pcb_ai_ui_set_status("AI ready");
    lv_timer_handler();
  } else {
    g_ai_ready = 0;
    printf("AI init failed: %s\r\n", app_ai_last_error());
    pcb_ai_ui_set_status("AI init failed");
    lv_timer_handler();
  }

  printf("Classifier init start\r\n");
  if (app_tiny_classifier_init() == 0) {
    g_classifier_ready = 1;
    printf("Classifier init ok\r\n");
  } else {
    g_classifier_ready = 0;
    printf("Classifier init failed: %s\r\n", app_tiny_classifier_last_error());
  }

  pcb_ai_ui_set_status("Ready");
  preview_current_sample();
  lv_timer_handler();
  printf("main loop start\r\n");

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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

  /* External SDRAM: LVGL canvas, LTDC buffers and AI activations live here. */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* FMC LCD address window. */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64MB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
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
