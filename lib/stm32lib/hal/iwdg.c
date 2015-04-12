
#include "iwdg.h"

typedef enum {
  IWDG_KR_reset = 0xaaaa,
  IWDG_KR_writeEnable = 0x5555,
  IWDG_KR_writeDisable = 0x0000,
  IWDG_KR_start = 0xcccc
} IWDG_KR;

void IWDG_HAL_setup(IWDG_Prescaler prescaler, uint16_t timeout) {
  IWDG->KR = IWDG_KR_writeEnable;
  IWDG->PR = prescaler;
  IWDG->RLR = timeout;
  IWDG->KR = IWDG_KR_writeDisable;
}

void IWDG_reset() {
  IWDG->KR = IWDG_KR_reset;
}

void IWDG_enable() {
  IWDG->KR = IWDG_KR_start;
}

