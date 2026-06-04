#include "app_ai.h"

#include <stdio.h>
#include <string.h>

#include "ai_platform.h"
#include "network.h"
#include "network_data.h"
#include "network_data_params.h"
#include "stm32h7xx_hal.h"

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define APP_AI_SDRAM_ATTR __attribute__((section(".sdram_ai"), used, aligned(32)))
#else
#define APP_AI_SDRAM_ATTR
#endif

APP_AI_SDRAM_ATTR
static ai_u8 s_ai_activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

static ai_handle s_ai_network = AI_HANDLE_NULL;
static ai_buffer *s_ai_input = NULL;
static ai_buffer *s_ai_output = NULL;
static const float *s_ai_output_data = NULL;
static char s_ai_last_error[96] = "not initialized";

static ai_handle s_ai_activations_map[AI_NETWORK_DATA_ACTIVATIONS_COUNT] = {
  AI_HANDLE_PTR(s_ai_activations),
};

static void app_ai_set_error(const char *stage, ai_error err)
{
  (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error),
                 "%s failed: type=0x%02lx code=0x%06lx",
                 stage, (unsigned long)err.type, (unsigned long)err.code);
}

int app_ai_init(void)
{
  ai_error err;
  ai_u16 n_inputs = 0;
  ai_u16 n_outputs = 0;

  if (s_ai_network != AI_HANDLE_NULL) {
    return 0;
  }

  err = ai_network_create_and_init(&s_ai_network, s_ai_activations_map, NULL);
  if (err.type != AI_ERROR_NONE) {
    app_ai_set_error("ai_network_create_and_init", err);
    s_ai_network = AI_HANDLE_NULL;
    return -1;
  }

  s_ai_input = ai_network_inputs_get(s_ai_network, &n_inputs);
  s_ai_output = ai_network_outputs_get(s_ai_network, &n_outputs);
  if ((s_ai_input == NULL) || (s_ai_output == NULL) ||
      (n_inputs != AI_NETWORK_IN_NUM) || (n_outputs != AI_NETWORK_OUT_NUM) ||
      (s_ai_input[0].data == AI_HANDLE_NULL) || (s_ai_output[0].data == AI_HANDLE_NULL)) {
    (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error),
                   "invalid io buffers: in=%u out=%u",
                   (unsigned)n_inputs, (unsigned)n_outputs);
    return -1;
  }

  s_ai_output_data = (const float *)s_ai_output[0].data;
  (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error), "ok");
  return 0;
}

int app_ai_run(const float *input, float *score, uint32_t *latency_ms)
{
  float *input_data;
  const float *output_data;
  uint32_t tick_start;
  ai_i32 batch;
  double sum_sq = 0.0;

  if ((s_ai_network == AI_HANDLE_NULL) || (s_ai_input == NULL) || (s_ai_output == NULL)) {
    (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error), "network not initialized");
    return -1;
  }
  if ((input == NULL) || (score == NULL) || (latency_ms == NULL)) {
    (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error), "null argument");
    return -1;
  }
  if ((AI_NETWORK_IN_1_SIZE != AI_NETWORK_OUT_1_SIZE) ||
      (AI_NETWORK_IN_1_SIZE != (3U * 96U * 96U)) ||
      (AI_NETWORK_IN_1_FORMAT != AI_BUFFER_FORMAT_FLOAT) ||
      (AI_NETWORK_OUT_1_FORMAT != AI_BUFFER_FORMAT_FLOAT)) {
    (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error), "unexpected network io format/size");
    return -1;
  }

  input_data = (float *)s_ai_input[0].data;
  output_data = (const float *)s_ai_output[0].data;

  memcpy(input_data, input, AI_NETWORK_IN_1_SIZE_BYTES);

  tick_start = HAL_GetTick();
  batch = ai_network_run(s_ai_network, s_ai_input, s_ai_output);
  *latency_ms = HAL_GetTick() - tick_start;
  if (batch != 1) {
    ai_error err = ai_network_get_error(s_ai_network);
    app_ai_set_error("ai_network_run", err);
    return -1;
  }

  for (uint32_t i = 0; i < AI_NETWORK_OUT_1_SIZE; i++) {
    const double diff = (double)input[i] - (double)output_data[i];
    sum_sq += diff * diff;
  }

  *score = (float)(sum_sq / (double)AI_NETWORK_OUT_1_SIZE);
  s_ai_output_data = output_data;
  (void)snprintf(s_ai_last_error, sizeof(s_ai_last_error), "ok");
  return 0;
}

const float *app_ai_get_output(void)
{
  return s_ai_output_data;
}

const char *app_ai_last_error(void)
{
  return s_ai_last_error;
}
