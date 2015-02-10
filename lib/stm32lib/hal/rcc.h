
#ifndef _STM32LIB_HAL_RCC_H_
#define _STM32LIB_HAL_RCC_H_

#include "base.h"

#define RCC_peripheral2_GPIOA  0  // TODO
#define RCC_peripheral2_AFIO   0  // TODO
#define RCC_peripheral2_USART1 0  // TODO

void RCC_peripheral1ClockEnable(uint32_t set);
void RCC_peripheral2ClockEnable(uint32_t set);

void RCC_peripheral1Clock(enableDisable state, uint32_t set);
void RCC_peripheral2Clock(enableDisable state, uint32_t set);

#endif
