
#include "nvic.h"
#include "chip/chip.h"

void NVIC_enable(NVIC_InitParams *initParams) {
  assert_param(IS_NVIC_PRIORITY(initParams->priority));

  NVIC_SetPriority(initParams->channel, initParams->priority);
  NVIC_EnableIRQ(initParams->channel);
}

