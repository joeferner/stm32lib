
#ifndef _STM32LIB_HAL_DAC_H_
#define _STM32LIB_HAL_DAC_H_

#include "base.h"

typedef enum {
  DAC_Channel_1 = 0x00000000,
  DAC_Channel_2 = 0x00000010
} DAC_Channel;
#define IS_DAC_CHANNEL(ch) ( \
  ((ch) == DAC_Channel_1) \
  || ((ch) == DAC_Channel_2) \
)

typedef enum {
  DAC_OutputBuffer_enable = 0x00000000,
  DAC_OutputBuffer_disable = DAC_CR_BOFF1
} DAC_OutputBuffer;
#define DAC_OutputBuffer_mask DAC_CR_BOFF1
#define IS_DAC_OUTPUT_BUFFER(b) ( \
  ((b) == DAC_OutputBuffer_enable) \
  || ((b) == DAC_OutputBuffer_disable) \
)

typedef enum {
  DAC_Alignment_12bitRight,
  DAC_Alignment_12bitLeft,
  DAC_Alignment_8bitRight
} DAC_Alignment;
#define IS_DAC_ALIGNMENT(a) ( \
  ((a) == DAC_Alignment_12bitRight) \
  || ((a) == DAC_Alignment_12bitLeft) \
  || ((a) == DAC_Alignment_8bitRight) \
)

typedef struct {
  DAC_Channel channel;
  DAC_OutputBuffer outputBuffer;
} HAL_DAC_InitParams;

void HAL_DAC_initParamsInit(HAL_DAC_InitParams *initParams);
void HAL_DAC_init(HAL_DAC_InitParams *initParams);
void DAC_enable(DAC_Channel ch);
void DAC_disable(DAC_Channel ch);
void DAC_setState(DAC_Channel ch, FunctionalState state);
void DAC_set(DAC_Channel ch, DAC_Alignment alignment, uint16_t d);

#endif
