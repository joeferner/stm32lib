
#include "platform_config.h"
#include <stdio.h>

extern volatile EXTI_Line gpioInterruptLine;
extern volatile IRQn_Type gpioIrq;

extern void onExti();

void SysTick_Handler() {
  time_SysTick_Handler();
}

void EXTI4_15_IRQHandler() {
  GPIO_TypeDef *gpioa = GPIOA;
  GPIO_TypeDef *gpioc = GPIOC;
  RCC_TypeDef *rcc = RCC;
  SYSCFG_TypeDef *syscfg = SYSCFG;
  EXTI_TypeDef *exti = EXTI;
  NVIC_Type *nvic = NVIC;
  uint32_t primask = __get_PRIMASK();
  uint32_t pendingIrq = NVIC_GetPendingIRQ(gpioIrq);

  if (EXTI_getStatus(gpioInterruptLine) != RESET) {
    EXTI_clearPendingBit(gpioInterruptLine);
    onExti();
    NVIC_ClearPendingIRQ(gpioIrq);
  }
}

