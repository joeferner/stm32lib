
#ifndef _STM32LIB_HAL_RCC_H_
#define _STM32LIB_HAL_RCC_H_

#include "base.h"

#define RCC_peripheral_AFIO        (1<<0)
#define RCC_peripheral_GPIOA       (1<<1)
#define RCC_peripheral_GPIOB       (1<<2)
#define RCC_peripheral_GPIOC       (1<<3)
#define RCC_peripheral_GPIOD       (1<<4)
#define RCC_peripheral_GPIOE       (1<<5)
#define RCC_peripheral_GPIOF       (1<<6)
#define RCC_peripheral_DAC         (1<<7)
#define RCC_peripheral_USART1      (1<<8)
#define RCC_peripheral_SPI1        (1<<9)
#define IS_RCC_PERIPHERAL(v) (v & ((1 << 10) - 1))

void RCC_peripheralClockEnable(uint64_t set);
void RCC_peripheralClock(uint64_t set, FunctionalState state);

uint32_t RCC_getPCLK1Freq();
uint32_t RCC_getHCLKFreq();
uint32_t RCC_getSysClockFreq();

#endif
