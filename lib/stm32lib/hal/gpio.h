
#ifndef _STM32LIB_HAL_GPIO_H_
#define _STM32LIB_HAL_GPIO_H_

#include "base.h"

typedef GPIO_TypeDef* GPIO_Instance;

typedef enum {
  GPIO_Pin_9 = 9,
  GPIO_Pin_10 = 10
} GPIO_Pin;

typedef enum {
  GPIO_Mode_input = 1, // TODO
  GPIO_Mode_outputPushPull = 1 // TODO
} GPIO_Mode;

typedef enum {
  GPIO_Speed_high = 3 // TODO
} GPIO_Speed;

typedef struct {
  GPIO_Instance port;
  GPIO_Pin pin;
  GPIO_Mode mode;
  GPIO_Speed speed;
} GPIO_InitParams;

void GPIO_initParamsInit(GPIO_InitParams* initParams);
void GPIO_init(GPIO_InitParams* initParams);

#endif
