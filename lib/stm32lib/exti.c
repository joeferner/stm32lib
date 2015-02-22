
#include "exti.h"
#include "hal/gpio.h"

EXTI_Line EXTI_getLineForGpio(GPIO_Port port, GPIO_Pin pin) {
  assert_param(IS_GPIO_PIN(pin));

  switch (pin) {
  case GPIO_Pin_0: return EXTI_Line_0;
  case GPIO_Pin_1: return EXTI_Line_1;
  case GPIO_Pin_2: return EXTI_Line_2;
  case GPIO_Pin_3: return EXTI_Line_3;
  case GPIO_Pin_4: return EXTI_Line_4;
  case GPIO_Pin_5: return EXTI_Line_5;
  case GPIO_Pin_6: return EXTI_Line_6;
  case GPIO_Pin_7: return EXTI_Line_7;
  case GPIO_Pin_8: return EXTI_Line_8;
  case GPIO_Pin_9: return EXTI_Line_9;
  case GPIO_Pin_10: return EXTI_Line_10;
  case GPIO_Pin_11: return EXTI_Line_11;
  case GPIO_Pin_12: return EXTI_Line_12;
  case GPIO_Pin_13: return EXTI_Line_13;
  case GPIO_Pin_14: return EXTI_Line_14;
  case GPIO_Pin_15: return EXTI_Line_15;
  }
  assert_param(0);
  return -1;
}

IRQn_Type EXTI_getIRQForGpio(GPIO_Port port, GPIO_Pin pin) {
  assert_param(IS_GPIO_PIN(pin));

  switch (pin) {
  case GPIO_Pin_0: return EXTI0_1_IRQn;
  case GPIO_Pin_1: return EXTI0_1_IRQn;
  case GPIO_Pin_2: return EXTI2_3_IRQn;
  case GPIO_Pin_3: return EXTI2_3_IRQn;
  case GPIO_Pin_4: return EXTI4_15_IRQn;
  case GPIO_Pin_5: return EXTI4_15_IRQn;
  case GPIO_Pin_6: return EXTI4_15_IRQn;
  case GPIO_Pin_7: return EXTI4_15_IRQn;
  case GPIO_Pin_8: return EXTI4_15_IRQn;
  case GPIO_Pin_9: return EXTI4_15_IRQn;
  case GPIO_Pin_10: return EXTI4_15_IRQn;
  case GPIO_Pin_11: return EXTI4_15_IRQn;
  case GPIO_Pin_12: return EXTI4_15_IRQn;
  case GPIO_Pin_13: return EXTI4_15_IRQn;
  case GPIO_Pin_14: return EXTI4_15_IRQn;
  case GPIO_Pin_15: return EXTI4_15_IRQn;
  }
  assert_param(0);
  return -1;
}
