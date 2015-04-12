

#ifndef _STM32LIB_HAL_IWDG_H_
#define _STM32LIB_HAL_IWDG_H_

#include "base.h"

typedef enum {
  IWDG_Prescaler_4 = 0b000,
  IWDG_Prescaler_8 = 0b001,
  IWDG_Prescaler_16 = 0b010,
  IWDG_Prescaler_32 = 0b011,
  IWDG_Prescaler_64 = 0b100,
  IWDG_Prescaler_128 = 0b101,
  IWDG_Prescaler_256 = 0b110
} IWDG_Prescaler;

void IWDG_HAL_setup(IWDG_Prescaler prescaler, uint16_t timeout);
void IWDG_reset();
void IWDG_enable();

#endif
