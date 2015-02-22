
#ifndef _STM32LIB_HAL_NVIC_H_
#define _STM32LIB_HAL_NVIC_H_

#include "base.h"

typedef IRQn_Type NVIC_Channel;

typedef struct {
  NVIC_Channel channel;
  uint8_t priority;
} NVIC_InitParams;

#define IS_NVIC_PRIORITY(p)  ((p) < 0x04)

void NVIC_enable(NVIC_InitParams *initParams);

#endif

