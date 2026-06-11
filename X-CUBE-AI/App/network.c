/**
  ******************************************************************************
  * @file    network.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-06-05T10:06:16+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */


#include "network.h"
#include "network_data.h"

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "ai_math_helpers.h"

#include "core_common.h"
#include "core_convert.h"

#include "layers.h"



#undef AI_NET_OBJ_INSTANCE
#define AI_NET_OBJ_INSTANCE g_network
 
#undef AI_NETWORK_MODEL_SIGNATURE
#define AI_NETWORK_MODEL_SIGNATURE     "0x97ee2a65448f2b9045958ff8949da962"

#ifndef AI_TOOLS_REVISION_ID
#define AI_TOOLS_REVISION_ID     ""
#endif

#undef AI_TOOLS_DATE_TIME
#define AI_TOOLS_DATE_TIME   "2026-06-05T10:06:16+0800"

#undef AI_TOOLS_COMPILE_TIME
#define AI_TOOLS_COMPILE_TIME    __DATE__ " " __TIME__

#undef AI_NETWORK_N_BATCHES
#define AI_NETWORK_N_BATCHES         (1)

static ai_ptr g_network_activations_map[1] = AI_C_ARRAY_INIT;
static ai_ptr g_network_weights_map[1] = AI_C_ARRAY_INIT;



/**  Array declarations section  **********************************************/
/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  input_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 27648, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  input_Transpose_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_1_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 13824, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_3_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 13824, AI_STATIC)

/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6912, AI_STATIC)

/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_5_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6912, AI_STATIC)

/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_upsample_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 25392, AI_STATIC)

/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 13824, AI_STATIC)

/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_1_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 13824, AI_STATIC)

/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_upsample_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 53016, AI_STATIC)

/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_3_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_upsample_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 108300, AI_STATIC)

/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  output_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27648, AI_STATIC)

/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  output_Transpose_0_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 27648, AI_STATIC)

/* Array#18 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 324, AI_STATIC)

/* Array#19 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12, AI_STATIC)

/* Array#20 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 2592, AI_STATIC)

/* Array#21 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#22 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 10368, AI_STATIC)

/* Array#23 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 48, AI_STATIC)

/* Array#24 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 18432, AI_STATIC)

/* Array#25 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#26 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4608, AI_STATIC)

/* Array#27 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 12, AI_STATIC)

/* Array#28 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 576, AI_STATIC)

/* Array#29 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3, AI_STATIC)

/* Array#30 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 27, AI_STATIC)

/* Array#31 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 108, AI_STATIC)

/* Array#32 */
AI_ARRAY_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 216, AI_STATIC)

/* Array#33 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 768, AI_STATIC)

/* Array#34 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 384, AI_STATIC)

/* Array#35 */
AI_ARRAY_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 192, AI_STATIC)

/**  Tensor declarations section  *********************************************/
/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_bias, AI_STATIC,
  0, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_decoder_decoder_0_ConvTranspose_output_0_bias_array, NULL)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_output, AI_STATIC,
  1, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 24, 24), AI_STRIDE_INIT(4, 4, 4, 96, 2304),
  1, &_decoder_decoder_0_ConvTranspose_output_0_output_array, NULL)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_scratch0, AI_STATIC,
  2, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 4, 4), AI_STRIDE_INIT(4, 4, 4, 192, 768),
  1, &_decoder_decoder_0_ConvTranspose_output_0_scratch0_array, NULL)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_upsample_output, AI_STATIC,
  3, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 23, 23), AI_STRIDE_INIT(4, 4, 4, 192, 4416),
  1, &_decoder_decoder_0_ConvTranspose_output_0_upsample_output_array, NULL)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_weights, AI_STATIC,
  4, 0x0,
  AI_SHAPE_INIT(4, 48, 4, 4, 24), AI_STRIDE_INIT(4, 4, 192, 4608, 18432),
  1, &_decoder_decoder_0_ConvTranspose_output_0_weights_array, NULL)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_1_Relu_output_0_output, AI_STATIC,
  5, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 24, 24), AI_STRIDE_INIT(4, 4, 4, 96, 2304),
  1, &_decoder_decoder_1_Relu_output_0_output_array, NULL)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_bias, AI_STATIC,
  6, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 1, 1), AI_STRIDE_INIT(4, 4, 4, 48, 48),
  1, &_decoder_decoder_2_ConvTranspose_output_0_bias_array, NULL)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_output, AI_STATIC,
  7, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 48, 48), AI_STRIDE_INIT(4, 4, 4, 48, 2304),
  1, &_decoder_decoder_2_ConvTranspose_output_0_output_array, NULL)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_scratch0, AI_STATIC,
  8, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 4, 4), AI_STRIDE_INIT(4, 4, 4, 96, 384),
  1, &_decoder_decoder_2_ConvTranspose_output_0_scratch0_array, NULL)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_upsample_output, AI_STATIC,
  9, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 47, 47), AI_STRIDE_INIT(4, 4, 4, 96, 4512),
  1, &_decoder_decoder_2_ConvTranspose_output_0_upsample_output_array, NULL)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_weights, AI_STATIC,
  10, 0x0,
  AI_SHAPE_INIT(4, 24, 4, 4, 12), AI_STRIDE_INIT(4, 4, 96, 1152, 4608),
  1, &_decoder_decoder_2_ConvTranspose_output_0_weights_array, NULL)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_3_Relu_output_0_output, AI_STATIC,
  11, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 48, 48), AI_STRIDE_INIT(4, 4, 4, 48, 2304),
  1, &_decoder_decoder_3_Relu_output_0_output_array, NULL)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_bias, AI_STATIC,
  12, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 1, 1), AI_STRIDE_INIT(4, 4, 4, 12, 12),
  1, &_decoder_decoder_4_ConvTranspose_output_0_bias_array, NULL)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_output, AI_STATIC,
  13, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 96, 96), AI_STRIDE_INIT(4, 4, 4, 12, 1152),
  1, &_decoder_decoder_4_ConvTranspose_output_0_output_array, NULL)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_scratch0, AI_STATIC,
  14, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 4, 4), AI_STRIDE_INIT(4, 4, 4, 48, 192),
  1, &_decoder_decoder_4_ConvTranspose_output_0_scratch0_array, NULL)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_upsample_output, AI_STATIC,
  15, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 95, 95), AI_STRIDE_INIT(4, 4, 4, 48, 4560),
  1, &_decoder_decoder_4_ConvTranspose_output_0_upsample_output_array, NULL)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_weights, AI_STATIC,
  16, 0x0,
  AI_SHAPE_INIT(4, 12, 4, 4, 3), AI_STRIDE_INIT(4, 4, 48, 144, 576),
  1, &_decoder_decoder_4_ConvTranspose_output_0_weights_array, NULL)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_bias, AI_STATIC,
  17, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 1, 1), AI_STRIDE_INIT(4, 4, 4, 48, 48),
  1, &_encoder_encoder_0_Conv_output_0_bias_array, NULL)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_output, AI_STATIC,
  18, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 48, 48), AI_STRIDE_INIT(4, 4, 4, 48, 2304),
  1, &_encoder_encoder_0_Conv_output_0_output_array, NULL)

/* Tensor #19 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_scratch0, AI_STATIC,
  19, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 3, 3), AI_STRIDE_INIT(4, 4, 4, 12, 36),
  1, &_encoder_encoder_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #20 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_weights, AI_STATIC,
  20, 0x0,
  AI_SHAPE_INIT(4, 3, 3, 3, 12), AI_STRIDE_INIT(4, 4, 12, 144, 432),
  1, &_encoder_encoder_0_Conv_output_0_weights_array, NULL)

/* Tensor #21 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_1_Relu_output_0_output, AI_STATIC,
  21, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 48, 48), AI_STRIDE_INIT(4, 4, 4, 48, 2304),
  1, &_encoder_encoder_1_Relu_output_0_output_array, NULL)

/* Tensor #22 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_bias, AI_STATIC,
  22, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_encoder_encoder_2_Conv_output_0_bias_array, NULL)

/* Tensor #23 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_output, AI_STATIC,
  23, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 24, 24), AI_STRIDE_INIT(4, 4, 4, 96, 2304),
  1, &_encoder_encoder_2_Conv_output_0_output_array, NULL)

/* Tensor #24 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_scratch0, AI_STATIC,
  24, 0x0,
  AI_SHAPE_INIT(4, 1, 12, 3, 3), AI_STRIDE_INIT(4, 4, 4, 48, 144),
  1, &_encoder_encoder_2_Conv_output_0_scratch0_array, NULL)

/* Tensor #25 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_weights, AI_STATIC,
  25, 0x0,
  AI_SHAPE_INIT(4, 12, 3, 3, 24), AI_STRIDE_INIT(4, 4, 48, 1152, 3456),
  1, &_encoder_encoder_2_Conv_output_0_weights_array, NULL)

/* Tensor #26 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_3_Relu_output_0_output, AI_STATIC,
  26, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 24, 24), AI_STRIDE_INIT(4, 4, 4, 96, 2304),
  1, &_encoder_encoder_3_Relu_output_0_output_array, NULL)

/* Tensor #27 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_bias, AI_STATIC,
  27, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 1, 1), AI_STRIDE_INIT(4, 4, 4, 192, 192),
  1, &_encoder_encoder_4_Conv_output_0_bias_array, NULL)

/* Tensor #28 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_output, AI_STATIC,
  28, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 12, 12), AI_STRIDE_INIT(4, 4, 4, 192, 2304),
  1, &_encoder_encoder_4_Conv_output_0_output_array, NULL)

/* Tensor #29 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_scratch0, AI_STATIC,
  29, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 3, 3), AI_STRIDE_INIT(4, 4, 4, 96, 288),
  1, &_encoder_encoder_4_Conv_output_0_scratch0_array, NULL)

/* Tensor #30 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_weights, AI_STATIC,
  30, 0x0,
  AI_SHAPE_INIT(4, 24, 3, 3, 48), AI_STRIDE_INIT(4, 4, 96, 4608, 13824),
  1, &_encoder_encoder_4_Conv_output_0_weights_array, NULL)

/* Tensor #31 */
AI_TENSOR_OBJ_DECLARE(
  _encoder_encoder_5_Relu_output_0_output, AI_STATIC,
  31, 0x0,
  AI_SHAPE_INIT(4, 1, 48, 12, 12), AI_STRIDE_INIT(4, 4, 4, 192, 2304),
  1, &_encoder_encoder_5_Relu_output_0_output_array, NULL)

/* Tensor #32 */
AI_TENSOR_OBJ_DECLARE(
  input_Transpose_output, AI_STATIC,
  32, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 96, 96), AI_STRIDE_INIT(4, 4, 4, 12, 1152),
  1, &input_Transpose_output_array, NULL)

/* Tensor #33 */
AI_TENSOR_OBJ_DECLARE(
  input_output, AI_STATIC,
  33, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 96, 3), AI_STRIDE_INIT(4, 4, 4, 384, 36864),
  1, &input_output_array, NULL)

/* Tensor #34 */
AI_TENSOR_OBJ_DECLARE(
  output_Transpose_0_output, AI_STATIC,
  34, 0x0,
  AI_SHAPE_INIT(4, 1, 96, 96, 3), AI_STRIDE_INIT(4, 4, 4, 384, 36864),
  1, &output_Transpose_0_output_array, NULL)

/* Tensor #35 */
AI_TENSOR_OBJ_DECLARE(
  output_output, AI_STATIC,
  35, 0x0,
  AI_SHAPE_INIT(4, 1, 3, 96, 96), AI_STRIDE_INIT(4, 4, 4, 12, 1152),
  1, &output_output_array, NULL)



/**  Layer declarations section  **********************************************/


AI_TENSOR_CHAIN_OBJ_DECLARE(
  output_Transpose_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &output_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &output_Transpose_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  output_Transpose_0_layer, 1,
  TRANSPOSE_TYPE, 0x0, NULL,
  transpose, forward_transpose,
  &output_Transpose_0_chain,
  NULL, &output_Transpose_0_layer, AI_STATIC, 
  .out_mapping = AI_SHAPE_INIT(6, AI_SHAPE_IN_CHANNEL, AI_SHAPE_WIDTH, AI_SHAPE_HEIGHT, AI_SHAPE_CHANNEL, AI_SHAPE_DEPTH, AI_SHAPE_EXTENSION), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  output_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_4_ConvTranspose_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &output_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  output_layer, 12,
  NL_TYPE, 0x0, NULL,
  nl, forward_sigmoid,
  &output_chain,
  NULL, &output_Transpose_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_4_ConvTranspose_output_0_upsample_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_4_ConvTranspose_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_decoder_decoder_4_ConvTranspose_output_0_weights, &_decoder_decoder_4_ConvTranspose_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_decoder_decoder_4_ConvTranspose_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_layer, 11,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_decoder_decoder_4_ConvTranspose_output_0_chain,
  NULL, &output_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)


AI_STATIC_CONST ai_float _decoder_decoder_4_ConvTranspose_output_0_upsample_scales_data[] = { 2, 2, 1.0, 1.0 };
AI_ARRAY_OBJ_DECLARE(
    _decoder_decoder_4_ConvTranspose_output_0_upsample_scales, AI_ARRAY_FORMAT_FLOAT,
    _decoder_decoder_4_ConvTranspose_output_0_upsample_scales_data, _decoder_decoder_4_ConvTranspose_output_0_upsample_scales_data, 4, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_upsample_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_3_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_4_ConvTranspose_output_0_upsample_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_4_ConvTranspose_output_0_upsample_layer, 11,
  UPSAMPLE_TYPE, 0x0, NULL,
  upsample, forward_upsample_zeros,
  &_decoder_decoder_4_ConvTranspose_output_0_upsample_chain,
  NULL, &_decoder_decoder_4_ConvTranspose_output_0_layer, AI_STATIC, 
  .scales = &_decoder_decoder_4_ConvTranspose_output_0_upsample_scales, 
  .center = false, 
  .mode = AI_UPSAMPLE_ZEROS, 
  .nearest_mode = AI_ROUND_PREFER_CEIL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_3_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_2_ConvTranspose_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_3_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_3_Relu_output_0_layer, 10,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_decoder_decoder_3_Relu_output_0_chain,
  NULL, &_decoder_decoder_4_ConvTranspose_output_0_upsample_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_2_ConvTranspose_output_0_upsample_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_2_ConvTranspose_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_decoder_decoder_2_ConvTranspose_output_0_weights, &_decoder_decoder_2_ConvTranspose_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_decoder_decoder_2_ConvTranspose_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_layer, 9,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_decoder_decoder_2_ConvTranspose_output_0_chain,
  NULL, &_decoder_decoder_3_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)


AI_STATIC_CONST ai_float _decoder_decoder_2_ConvTranspose_output_0_upsample_scales_data[] = { 2, 2, 1.0, 1.0 };
AI_ARRAY_OBJ_DECLARE(
    _decoder_decoder_2_ConvTranspose_output_0_upsample_scales, AI_ARRAY_FORMAT_FLOAT,
    _decoder_decoder_2_ConvTranspose_output_0_upsample_scales_data, _decoder_decoder_2_ConvTranspose_output_0_upsample_scales_data, 4, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_upsample_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_1_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_2_ConvTranspose_output_0_upsample_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_2_ConvTranspose_output_0_upsample_layer, 9,
  UPSAMPLE_TYPE, 0x0, NULL,
  upsample, forward_upsample_zeros,
  &_decoder_decoder_2_ConvTranspose_output_0_upsample_chain,
  NULL, &_decoder_decoder_2_ConvTranspose_output_0_layer, AI_STATIC, 
  .scales = &_decoder_decoder_2_ConvTranspose_output_0_upsample_scales, 
  .center = false, 
  .mode = AI_UPSAMPLE_ZEROS, 
  .nearest_mode = AI_ROUND_PREFER_CEIL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_1_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_0_ConvTranspose_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_1_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_1_Relu_output_0_layer, 8,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_decoder_decoder_1_Relu_output_0_chain,
  NULL, &_decoder_decoder_2_ConvTranspose_output_0_upsample_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_0_ConvTranspose_output_0_upsample_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_0_ConvTranspose_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_decoder_decoder_0_ConvTranspose_output_0_weights, &_decoder_decoder_0_ConvTranspose_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_decoder_decoder_0_ConvTranspose_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_layer, 7,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_decoder_decoder_0_ConvTranspose_output_0_chain,
  NULL, &_decoder_decoder_1_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 2, 2, 2, 2), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)


AI_STATIC_CONST ai_float _decoder_decoder_0_ConvTranspose_output_0_upsample_scales_data[] = { 2, 2, 1.0, 1.0 };
AI_ARRAY_OBJ_DECLARE(
    _decoder_decoder_0_ConvTranspose_output_0_upsample_scales, AI_ARRAY_FORMAT_FLOAT,
    _decoder_decoder_0_ConvTranspose_output_0_upsample_scales_data, _decoder_decoder_0_ConvTranspose_output_0_upsample_scales_data, 4, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_upsample_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_5_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_decoder_decoder_0_ConvTranspose_output_0_upsample_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _decoder_decoder_0_ConvTranspose_output_0_upsample_layer, 7,
  UPSAMPLE_TYPE, 0x0, NULL,
  upsample, forward_upsample_zeros,
  &_decoder_decoder_0_ConvTranspose_output_0_upsample_chain,
  NULL, &_decoder_decoder_0_ConvTranspose_output_0_layer, AI_STATIC, 
  .scales = &_decoder_decoder_0_ConvTranspose_output_0_upsample_scales, 
  .center = false, 
  .mode = AI_UPSAMPLE_ZEROS, 
  .nearest_mode = AI_ROUND_PREFER_CEIL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _encoder_encoder_5_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_4_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_5_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _encoder_encoder_5_Relu_output_0_layer, 6,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_encoder_encoder_5_Relu_output_0_chain,
  NULL, &_decoder_decoder_0_ConvTranspose_output_0_upsample_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_3_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_4_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_encoder_encoder_4_Conv_output_0_weights, &_encoder_encoder_4_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_encoder_encoder_4_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _encoder_encoder_4_Conv_output_0_layer, 5,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_encoder_encoder_4_Conv_output_0_chain,
  NULL, &_encoder_encoder_5_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _encoder_encoder_3_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_3_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _encoder_encoder_3_Relu_output_0_layer, 4,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_encoder_encoder_3_Relu_output_0_chain,
  NULL, &_encoder_encoder_4_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_1_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_2_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_encoder_encoder_2_Conv_output_0_weights, &_encoder_encoder_2_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_encoder_encoder_2_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _encoder_encoder_2_Conv_output_0_layer, 3,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_encoder_encoder_2_Conv_output_0_chain,
  NULL, &_encoder_encoder_3_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _encoder_encoder_1_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_1_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _encoder_encoder_1_Relu_output_0_layer, 2,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_encoder_encoder_1_Relu_output_0_chain,
  NULL, &_encoder_encoder_2_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &input_Transpose_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_encoder_encoder_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_encoder_encoder_0_Conv_output_0_weights, &_encoder_encoder_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_encoder_encoder_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _encoder_encoder_0_Conv_output_0_layer, 1,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_encoder_encoder_0_Conv_output_0_chain,
  NULL, &_encoder_encoder_1_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  input_Transpose_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &input_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &input_Transpose_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  input_Transpose_layer, 2,
  TRANSPOSE_TYPE, 0x0, NULL,
  transpose, forward_transpose,
  &input_Transpose_chain,
  NULL, &_encoder_encoder_0_Conv_output_0_layer, AI_STATIC, 
  .out_mapping = AI_SHAPE_INIT(6, AI_SHAPE_IN_CHANNEL, AI_SHAPE_HEIGHT, AI_SHAPE_CHANNEL, AI_SHAPE_WIDTH, AI_SHAPE_DEPTH, AI_SHAPE_EXTENSION), 
)


#if (AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 148092, 1, 1),
    148092, NULL, NULL),
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 543792, 1, 1),
    543792, NULL, NULL),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_IN_NUM, &input_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_OUT_NUM, &output_Transpose_0_output),
  &input_Transpose_layer, 0xb2b3035d, NULL)

#else

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 148092, 1, 1),
      148092, NULL, NULL)
  ),
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 543792, 1, 1),
      543792, NULL, NULL)
  ),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_IN_NUM, &input_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_OUT_NUM, &output_Transpose_0_output),
  &input_Transpose_layer, 0xb2b3035d, NULL)

#endif	/*(AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)*/



/******************************************************************************/
AI_DECLARE_STATIC
ai_bool network_configure_activations(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_activations_map(g_network_activations_map, 1, params)) {
    /* Updating activations (byte) offsets */
    
    input_output_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    input_output_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    input_Transpose_output_array.data = AI_PTR(g_network_activations_map[0] + 172944);
    input_Transpose_output_array.data_start = AI_PTR(g_network_activations_map[0] + 172944);
    _encoder_encoder_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    _encoder_encoder_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    _encoder_encoder_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 169392);
    _encoder_encoder_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 169392);
    _encoder_encoder_1_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 169392);
    _encoder_encoder_1_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 169392);
    _encoder_encoder_2_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    _encoder_encoder_2_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    _encoder_encoder_2_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 62784);
    _encoder_encoder_2_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 62784);
    _encoder_encoder_3_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 118080);
    _encoder_encoder_3_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 118080);
    _encoder_encoder_4_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    _encoder_encoder_4_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    _encoder_encoder_4_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 63216);
    _encoder_encoder_4_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 63216);
    _encoder_encoder_5_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 90864);
    _encoder_encoder_5_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 90864);
    _decoder_decoder_0_ConvTranspose_output_0_upsample_output_array.data = AI_PTR(g_network_activations_map[0] + 118512);
    _decoder_decoder_0_ConvTranspose_output_0_upsample_output_array.data_start = AI_PTR(g_network_activations_map[0] + 118512);
    _decoder_decoder_0_ConvTranspose_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    _decoder_decoder_0_ConvTranspose_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    _decoder_decoder_0_ConvTranspose_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 220080);
    _decoder_decoder_0_ConvTranspose_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 220080);
    _decoder_decoder_1_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    _decoder_decoder_1_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    _decoder_decoder_2_ConvTranspose_output_0_upsample_output_array.data = AI_PTR(g_network_activations_map[0] + 117648);
    _decoder_decoder_2_ConvTranspose_output_0_upsample_output_array.data_start = AI_PTR(g_network_activations_map[0] + 117648);
    _decoder_decoder_2_ConvTranspose_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 62352);
    _decoder_decoder_2_ConvTranspose_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 62352);
    _decoder_decoder_2_ConvTranspose_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 110592);
    _decoder_decoder_2_ConvTranspose_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 110592);
    _decoder_decoder_3_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _decoder_decoder_3_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _decoder_decoder_4_ConvTranspose_output_0_upsample_output_array.data = AI_PTR(g_network_activations_map[0] + 110592);
    _decoder_decoder_4_ConvTranspose_output_0_upsample_output_array.data_start = AI_PTR(g_network_activations_map[0] + 110592);
    _decoder_decoder_4_ConvTranspose_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _decoder_decoder_4_ConvTranspose_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _decoder_decoder_4_ConvTranspose_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 107100);
    _decoder_decoder_4_ConvTranspose_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 107100);
    output_output_array.data = AI_PTR(g_network_activations_map[0] + 217692);
    output_output_array.data_start = AI_PTR(g_network_activations_map[0] + 217692);
    output_Transpose_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    output_Transpose_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_ACTIVATIONS);
  return false;
}




/******************************************************************************/
AI_DECLARE_STATIC
ai_bool network_configure_weights(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_weights_map(g_network_weights_map, 1, params)) {
    /* Updating weights (byte) offsets */
    
    _encoder_encoder_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _encoder_encoder_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 0);
    _encoder_encoder_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 0);
    _encoder_encoder_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _encoder_encoder_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 1296);
    _encoder_encoder_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 1296);
    _encoder_encoder_2_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _encoder_encoder_2_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 1344);
    _encoder_encoder_2_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 1344);
    _encoder_encoder_2_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _encoder_encoder_2_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 11712);
    _encoder_encoder_2_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 11712);
    _encoder_encoder_4_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _encoder_encoder_4_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 11808);
    _encoder_encoder_4_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 11808);
    _encoder_encoder_4_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _encoder_encoder_4_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 53280);
    _encoder_encoder_4_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 53280);
    _decoder_decoder_0_ConvTranspose_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _decoder_decoder_0_ConvTranspose_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 53472);
    _decoder_decoder_0_ConvTranspose_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 53472);
    _decoder_decoder_0_ConvTranspose_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _decoder_decoder_0_ConvTranspose_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 127200);
    _decoder_decoder_0_ConvTranspose_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 127200);
    _decoder_decoder_2_ConvTranspose_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _decoder_decoder_2_ConvTranspose_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 127296);
    _decoder_decoder_2_ConvTranspose_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 127296);
    _decoder_decoder_2_ConvTranspose_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _decoder_decoder_2_ConvTranspose_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 145728);
    _decoder_decoder_2_ConvTranspose_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 145728);
    _decoder_decoder_4_ConvTranspose_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _decoder_decoder_4_ConvTranspose_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 145776);
    _decoder_decoder_4_ConvTranspose_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 145776);
    _decoder_decoder_4_ConvTranspose_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _decoder_decoder_4_ConvTranspose_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 148080);
    _decoder_decoder_4_ConvTranspose_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 148080);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_WEIGHTS);
  return false;
}


/**  PUBLIC APIs SECTION  *****************************************************/



AI_DEPRECATED
AI_API_ENTRY
ai_bool ai_network_get_info(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_NETWORK_MODEL_NAME,
      .model_signature   = AI_NETWORK_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 30668667,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .params            = AI_STRUCT_INIT,
      .activations       = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0xb2b3035d,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}



AI_API_ENTRY
ai_bool ai_network_get_report(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_NETWORK_MODEL_NAME,
      .model_signature   = AI_NETWORK_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 30668667,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .map_signature     = AI_MAGIC_SIGNATURE,
      .map_weights       = AI_STRUCT_INIT,
      .map_activations   = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0xb2b3035d,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}


AI_API_ENTRY
ai_error ai_network_get_error(ai_handle network)
{
  return ai_platform_network_get_error(network);
}


AI_API_ENTRY
ai_error ai_network_create(
  ai_handle* network, const ai_buffer* network_config)
{
  return ai_platform_network_create(
    network, network_config, 
    AI_CONTEXT_OBJ(&AI_NET_OBJ_INSTANCE),
    AI_TOOLS_API_VERSION_MAJOR, AI_TOOLS_API_VERSION_MINOR, AI_TOOLS_API_VERSION_MICRO);
}


AI_API_ENTRY
ai_error ai_network_create_and_init(
  ai_handle* network, const ai_handle activations[], const ai_handle weights[])
{
  ai_error err;
  ai_network_params params;

  err = ai_network_create(network, AI_NETWORK_DATA_CONFIG);
  if (err.type != AI_ERROR_NONE) {
    return err;
  }
  
  if (ai_network_data_params_get(&params) != true) {
    err = ai_network_get_error(*network);
    return err;
  }
#if defined(AI_NETWORK_DATA_ACTIVATIONS_COUNT)
  /* set the addresses of the activations buffers */
  for (ai_u16 idx=0; activations && idx<params.map_activations.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_activations, idx, activations[idx]);
  }
#endif
#if defined(AI_NETWORK_DATA_WEIGHTS_COUNT)
  /* set the addresses of the weight buffers */
  for (ai_u16 idx=0; weights && idx<params.map_weights.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_weights, idx, weights[idx]);
  }
#endif
  if (ai_network_init(*network, &params) != true) {
    err = ai_network_get_error(*network);
  }
  return err;
}


AI_API_ENTRY
ai_buffer* ai_network_inputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_inputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_buffer* ai_network_outputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_outputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_handle ai_network_destroy(ai_handle network)
{
  return ai_platform_network_destroy(network);
}


AI_API_ENTRY
ai_bool ai_network_init(
  ai_handle network, const ai_network_params* params)
{
  ai_network* net_ctx = AI_NETWORK_OBJ(ai_platform_network_init(network, params));
  ai_bool ok = true;

  if (!net_ctx) return false;
  ok &= network_configure_weights(net_ctx, params);
  ok &= network_configure_activations(net_ctx, params);

  ok &= ai_platform_network_post_init(network);

  return ok;
}


AI_API_ENTRY
ai_i32 ai_network_run(
  ai_handle network, const ai_buffer* input, ai_buffer* output)
{
  return ai_platform_network_process(network, input, output);
}


AI_API_ENTRY
ai_i32 ai_network_forward(ai_handle network, const ai_buffer* input)
{
  return ai_platform_network_process(network, input, NULL);
}



#undef AI_NETWORK_MODEL_SIGNATURE
#undef AI_NET_OBJ_INSTANCE
#undef AI_TOOLS_DATE_TIME
#undef AI_TOOLS_COMPILE_TIME

