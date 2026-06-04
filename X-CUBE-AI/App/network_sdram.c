/**
  ******************************************************************************
  * @file    network_sdram.c
  * @brief   SDRAM-backed activation buffer for the generated X-CUBE-AI network.
  ******************************************************************************
  */

#include "network_sdram.h"
#include "network.h"

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define AI_SDRAM_ACTIVATIONS_ATTR __attribute__((section(".ai_activations"), used, aligned(32)))
#else
#define AI_SDRAM_ACTIVATIONS_ATTR
#endif

AI_SDRAM_ACTIVATIONS_ATTR
static ai_u8 s_network_sdram_activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

static ai_handle s_network_sdram_activations_table[AI_NETWORK_DATA_ACTIVATIONS_COUNT] = {
  AI_HANDLE_PTR(s_network_sdram_activations),
};

const ai_handle *network_sdram_activations_get(void)
{
  return s_network_sdram_activations_table;
}

ai_u8 *network_sdram_activations_buffer_get(void)
{
  return s_network_sdram_activations;
}

ai_u32 network_sdram_activations_size_get(void)
{
  return AI_NETWORK_DATA_ACTIVATIONS_SIZE;
}

ai_error network_sdram_create_and_init(ai_handle *network)
{
  return ai_network_create_and_init(network, network_sdram_activations_get(), NULL);
}
