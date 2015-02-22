
#include "exti.h"
#include "rcc.h"

void EXTI_initParamsInit(EXTI_InitParams *initParams) {
  initParams->line = -1;
  initParams->mode = EXTI_Mode_interrupt;
  initParams->trigger = EXTI_Trigger_both;
}

void EXTI_enable(EXTI_InitParams *initParams) {
  assert_param(IS_EXTI_MODE(initParams->mode));
  assert_param(IS_EXTI_TRIGGER(initParams->trigger));
  assert_param(IS_EXTI_LINE(initParams->line));

  RCC_peripheralClockEnable(RCC_Peripheral_SYSCFG);

  // IMR and EMR
  EXTI->IMR &= ~initParams->line;
  EXTI->EMR &= ~initParams->line;
  if (initParams->mode == EXTI_Mode_interrupt) {
    EXTI->IMR |= initParams->line;
  } else if (initParams->mode == EXTI_Mode_event) {
    EXTI->EMR |= initParams->line;
  }

  // RTSR and FTSR
  EXTI->RTSR &= ~initParams->line;
  EXTI->FTSR &= ~initParams->line;
  if (initParams->trigger == EXTI_Trigger_both) {
    EXTI->RTSR |= initParams->line;
    EXTI->FTSR |= initParams->line;
  } else if (initParams->trigger == EXTI_Trigger_falling) {
    EXTI->FTSR |= initParams->line;
  } else if (initParams->trigger == EXTI_Trigger_rising) {
    EXTI->RTSR |= initParams->line;
  }
}

FlagStatus EXTI_getStatus(EXTI_Line line) {
  assert_param(IS_EXTI_LINE(line));

  if ((EXTI->PR & line) != (uint32_t)RESET) {
    return SET;
  } else {
    return RESET;
  }
}

void EXTI_clearPendingBit(EXTI_Line line) {
  assert_param(IS_EXTI_LINE(line));

  EXTI->PR = line;
}



