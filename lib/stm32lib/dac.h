
#ifndef _STM32LIB_DAC_H_
#define _STM32LIB_DAC_H_

#include "hal/dac.h"
#include "hal/gpio.h"

typedef struct {
  HAL_DAC_InitParams halDacInitParams;
  GPIO_Port port;
  GPIO_Pin pin;
} DAC_InitParams;

void DAC_initParamsInit(DAC_InitParams *initParams);
void DAC_init(DAC_InitParams *initParams);
DAC_Channel DAC_getChannelFromPortAndPin(GPIO_Port port, GPIO_Pin pin);

#endif
