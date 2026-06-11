#include "app_tiny_classifier.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "stm32h7xx_hal.h"
#include "ai_platform.h"
#include "network_classifier.h"
#include "network_classifier_data.h"
#include "network_classifier_data_params.h"

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define APP_TINY_CLASSIFIER_SDRAM_ATTR __attribute__((section(".sdram_ai"), used, aligned(32)))
#else
#define APP_TINY_CLASSIFIER_SDRAM_ATTR
#endif

APP_TINY_CLASSIFIER_SDRAM_ATTR
static ai_u8 s_classifier_activations[AI_NETWORK_CLASSIFIER_DATA_ACTIVATIONS_SIZE];

static ai_handle s_classifier_network = AI_HANDLE_NULL;
static ai_buffer *s_classifier_input = NULL;
static ai_buffer *s_classifier_output = NULL;
static char s_classifier_last_error[128] = "not initialized";

static ai_handle s_classifier_activations_map[AI_NETWORK_CLASSIFIER_DATA_ACTIVATIONS_COUNT] = {
  AI_HANDLE_PTR(s_classifier_activations),
};

static const char *const s_class_names[TINY_CLASSIFIER_NUM_CLASSES] = {
  "copper",
  "mousebite",
  "open",
  "pin-hole",
  "short",
  "spur",
};

static void app_tiny_classifier_set_error(const char *stage, ai_error err)
{
  (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error),
                 "%s failed: type=0x%02lx code=0x%06lx",
                 stage, (unsigned long)err.type, (unsigned long)err.code);
}

static void app_tiny_classifier_softmax(const float *logits, float *probs)
{
  float max_logit = logits[0];
  float sum = 0.0f;

  for (uint32_t i = 1; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
    if (logits[i] > max_logit) {
      max_logit = logits[i];
    }
  }

  for (uint32_t i = 0; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
    probs[i] = expf(logits[i] - max_logit);
    sum += probs[i];
  }

  if (sum > 0.0f) {
    const float inv_sum = 1.0f / sum;
    for (uint32_t i = 0; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
      probs[i] *= inv_sum;
    }
  }
}

int app_tiny_classifier_init(void)
{
  ai_error err;
  ai_u16 n_inputs = 0;
  ai_u16 n_outputs = 0;

  if (s_classifier_network != AI_HANDLE_NULL) {
    return 0;
  }

  err = ai_network_classifier_create_and_init(
    &s_classifier_network,
    s_classifier_activations_map,
    NULL);
  if (err.type != AI_ERROR_NONE) {
    app_tiny_classifier_set_error("ai_network_classifier_create_and_init", err);
    s_classifier_network = AI_HANDLE_NULL;
    return -1;
  }

  s_classifier_input = ai_network_classifier_inputs_get(s_classifier_network, &n_inputs);
  s_classifier_output = ai_network_classifier_outputs_get(s_classifier_network, &n_outputs);
  if ((s_classifier_input == NULL) || (s_classifier_output == NULL) ||
      (n_inputs != AI_NETWORK_CLASSIFIER_IN_NUM) ||
      (n_outputs != AI_NETWORK_CLASSIFIER_OUT_NUM) ||
      (s_classifier_input[0].data == AI_HANDLE_NULL) ||
      (s_classifier_output[0].data == AI_HANDLE_NULL)) {
    (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error),
                   "invalid io buffers: in=%u out=%u",
                   (unsigned)n_inputs, (unsigned)n_outputs);
    return -1;
  }

  if ((AI_NETWORK_CLASSIFIER_IN_1_SIZE != TINY_CLASSIFIER_INPUT_SIZE) ||
      (AI_NETWORK_CLASSIFIER_OUT_1_SIZE != TINY_CLASSIFIER_NUM_CLASSES) ||
      (AI_NETWORK_CLASSIFIER_IN_1_FORMAT != AI_BUFFER_FORMAT_FLOAT) ||
      (AI_NETWORK_CLASSIFIER_OUT_1_FORMAT != AI_BUFFER_FORMAT_FLOAT)) {
    (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error),
                   "unexpected classifier io format/size");
    return -1;
  }

  (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error), "ok");
  return 0;
}

int app_tiny_classifier_run(
    const float *input,
    tiny_classifier_result_t *result,
    uint32_t *latency_ms)
{
  float *input_data;
  const float *output_data;
  uint32_t tick_start;
  ai_i32 batch;
  int top1_index = 0;

  if ((s_classifier_network == AI_HANDLE_NULL) ||
      (s_classifier_input == NULL) ||
      (s_classifier_output == NULL)) {
    (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error),
                   "network not initialized");
    return -1;
  }

  if ((input == NULL) || (result == NULL) || (latency_ms == NULL)) {
    (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error),
                   "null argument");
    return -1;
  }

  input_data = (float *)s_classifier_input[0].data;
  output_data = (const float *)s_classifier_output[0].data;
  memcpy(input_data, input, AI_NETWORK_CLASSIFIER_IN_1_SIZE_BYTES);

  tick_start = HAL_GetTick();
  batch = ai_network_classifier_run(
    s_classifier_network,
    s_classifier_input,
    s_classifier_output);
  *latency_ms = HAL_GetTick() - tick_start;
  if (batch != 1) {
    ai_error err = ai_network_classifier_get_error(s_classifier_network);
    app_tiny_classifier_set_error("ai_network_classifier_run", err);
    return -1;
  }

  for (uint32_t i = 0; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
    result->logits[i] = output_data[i];
  }
  app_tiny_classifier_softmax(result->logits, result->probs);

  for (uint32_t i = 1; i < TINY_CLASSIFIER_NUM_CLASSES; i++) {
    if (result->probs[i] > result->probs[top1_index]) {
      top1_index = (int)i;
    }
  }

  result->top1_index = top1_index;
  result->top1_logit = result->logits[top1_index];
  result->top1_prob = result->probs[top1_index];

  (void)snprintf(s_classifier_last_error, sizeof(s_classifier_last_error), "ok");
  return 0;
}

const char *app_tiny_classifier_last_error(void)
{
  return s_classifier_last_error;
}

const char *app_tiny_classifier_class_name(int index)
{
  if ((index < 0) || (index >= TINY_CLASSIFIER_NUM_CLASSES)) {
    return "unknown";
  }
  return s_class_names[index];
}
