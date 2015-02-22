
#include "rcc.h"

void RCC_peripheralClockEnableForPort(GPIO_Port port) {
  if (port == GPIOA) {
    RCC_peripheralClockEnable(RCC_Peripheral_GPIOA);
  } else if (port == GPIOB) {
    RCC_peripheralClockEnable(RCC_Peripheral_GPIOB);
  } else if (port == GPIOC) {
    RCC_peripheralClockEnable(RCC_Peripheral_GPIOC);
  } else if (port == GPIOD) {
    RCC_peripheralClockEnable(RCC_Peripheral_GPIOD);
  } else if (port == GPIOE) {
    RCC_peripheralClockEnable(RCC_Peripheral_GPIOE);
  }  else if (port == GPIOF) {
    RCC_peripheralClockEnable(RCC_Peripheral_GPIOF);
  } else {
    assert_param(0);
  }
}
