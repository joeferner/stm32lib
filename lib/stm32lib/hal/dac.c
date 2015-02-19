
#include "dac.h"

void HAL_DAC_initParamsInit(HAL_DAC_InitParams *initParams) {
  initParams->channel = -1;
  initParams->outputBuffer = DAC_OutputBuffer_enable;
}

void HAL_DAC_init(HAL_DAC_InitParams *initParams) {
  uint32_t tmp;
  assert_param(IS_DAC_CHANNEL(initParams->channel));
  assert_param(IS_DAC_OUTPUT_BUFFER(initParams->outputBuffer));

  tmp = DAC->CR;

  tmp &= ~(DAC_OutputBuffer_mask << initParams->channel);
  tmp |= (initParams->outputBuffer << initParams->channel);

  DAC->CR = tmp;
}

void DAC_enable(DAC_Channel ch) {
  assert_param(IS_DAC_CHANNEL(ch));
  DAC_setState(ch, ENABLE);
}

void DAC_disable(DAC_Channel ch) {
  assert_param(IS_DAC_CHANNEL(ch));
  DAC_setState(ch, DISABLE);
}

void DAC_setState(DAC_Channel ch, FunctionalState state) {
  uint32_t bit;
  assert_param(IS_DAC_CHANNEL(ch));
  assert_param(IS_FUNCTIONAL_STATE(state));

  if (ch == DAC_Channel_1) {
    bit = DAC_CR_EN1;
  } else if (ch == DAC_Channel_2) {
    bit = DAC_CR_EN2;
  } else {
    assert_param(0);
  }

  if (state != DISABLE) {
    DAC->CR |= bit;
  } else {
    DAC->CR &= ~bit;
  }
}

void DAC_set(DAC_Channel ch, DAC_Alignment alignment, uint16_t d) {
  assert_param(IS_DAC_CHANNEL(ch));
  assert_param(IS_DAC_ALIGNMENT(alignment));

  if (ch == DAC_Channel_1) {
    if (alignment == DAC_Alignment_12bitLeft) {
      DAC->DHR12L1 = d;
    } else if (alignment == DAC_Alignment_12bitRight) {
      DAC->DHR12R1 = d;
    } else if (alignment == DAC_Alignment_8bitRight) {
      DAC->DHR8R1 = d;
    }
  } else if (ch == DAC_Channel_2) {
    if (alignment == DAC_Alignment_12bitLeft) {
      DAC->DHR12L2 = d;
    } else if (alignment == DAC_Alignment_12bitRight) {
      DAC->DHR12R2 = d;
    } else if (alignment == DAC_Alignment_8bitRight) {
      DAC->DHR8R2 = d;
    }
  } else {
    assert_param(0);
  }
}
