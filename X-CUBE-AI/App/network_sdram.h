/**
  ******************************************************************************
  * @file    network_sdram.h
  * @brief   SDRAM-backed activation buffer for the generated X-CUBE-AI network.
  ******************************************************************************
  */

#ifndef NETWORK_SDRAM_H
#define NETWORK_SDRAM_H

#include "ai_platform.h"
#include "network_data_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Call sdram_init() before using these buffers to initialize the network. */
const ai_handle *network_sdram_activations_get(void);
ai_u8 *network_sdram_activations_buffer_get(void);
ai_u32 network_sdram_activations_size_get(void);
ai_error network_sdram_create_and_init(ai_handle *network);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_SDRAM_H */
