
#include "platform_config.h"
#include <stdio.h>

extern EXTI_Line gpioInterruptLine;

extern void onExti();

void SysTick_Handler() {
  time_SysTick_Handler();
}

void EXTI4_15_IRQHandler() {
  printf("b\n");
  if (EXTI_getStatus(gpioInterruptLine) != RESET) {
    onExti();
    EXTI_clearPendingBit(gpioInterruptLine);
  }
}
