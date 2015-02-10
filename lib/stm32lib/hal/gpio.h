
#ifndef _STM32LIB_HAL_GPIO_H_
#define _STM32LIB_HAL_GPIO_H_

#include "base.h"

typedef enum {
  GPIOA
} GPIO_Port;

typedef enum {
  GPIO_pin_9 = 9,
  GPIO_pin_10 = 10
} GPIO_pin;

typedef enum {
  GPIO_mode_input = 1, // TODO
  GPIO_mode_outputPushPull = 1 // TODO
} GPIO_mode;

typedef enum {
  GPIO_speed_high = 3 // TODO
} GPIO_speed;

typedef struct {
  GPIO_Port port;
  GPIO_pin pin;
  GPIO_mode mode;
  GPIO_speed speed;
} GPIO_initParams;

void GPIO_initParamsInit(GPIO_initParams* initParams);
void GPIO_init(GPIO_initParams* initParams);

#endif
