
#include "iwdg.h"
#include "hal/iwdg.h"
#include "utils.h"

void IWDG_setup(uint32_t ms) {
  IWDG_Prescaler prescaler = IWDG_Prescaler_256;
  uint16_t timeout = 0xfff;

  if (ms <= 409) {
    prescaler = IWDG_Prescaler_4;
    timeout = (ms * 0xfff) / 409;
  } else if (ms <= 819) {
    prescaler = IWDG_Prescaler_8;
    timeout = (ms * 0xfff) / 819;
  } else if (ms <= 1638) {
    prescaler = IWDG_Prescaler_16;
    timeout = (ms * 0xfff) / 1638;
  } else if (ms <= 3276) {
    prescaler = IWDG_Prescaler_32;
    timeout = (ms * 0xfff) / 3276;
  } else if (ms <= 6553) {
    prescaler = IWDG_Prescaler_64;
    timeout = (ms * 0xfff) / 6553;
  } else if (ms <= 13107) {
    prescaler = IWDG_Prescaler_128;
    timeout = (ms * 0xfff) / 13107;
  } else if (ms <= 26215) {
    prescaler = IWDG_Prescaler_256;
    timeout = (ms * 0xfff) / 26215;
  } else {
    assert_param(0);
  }

  timeout = clamp(timeout, 0x000, 0xfff);
  printf("IWDG setup: 0x%1x, 0x%03x\n", prescaler, timeout);
  IWDG_HAL_setup(prescaler, timeout);
}
