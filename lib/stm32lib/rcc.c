
#include "rcc.h"

void RCC_peripheralClockEnableForPort(GPIO_Port port) {
  if (port == GPIOA) {
    RCC_peripheralClockEnable(RCC_peripheral_GPIOA);
  } else {
    assert_param(0);
  }
}
