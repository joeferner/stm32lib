
#include "platform_config.h"
#include <stdio.h>

extern volatile EXTI_Line gpioInterruptLine;
extern volatile IRQn_Type gpioIrq;

extern void onExti();

void SysTick_Handler() {
  time_SysTick_Handler();
}

void EXTI4_15_IRQHandler() {
  if (EXTI_getStatus(gpioInterruptLine) != RESET) {
    EXTI_clearPendingBit(gpioInterruptLine);
    onExti();
    NVIC_ClearPendingIRQ(gpioIrq);
  }
}

