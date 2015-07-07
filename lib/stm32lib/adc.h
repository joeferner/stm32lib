
#ifndef _STM32LIB_ADC_H_
#define _STM32LIB_ADC_H_

#include "hal/adc.h"
#include "hal/gpio.h"

typedef struct {
  HAL_ADC_InitParams halAdcInitParams;
  GPIO_Port port;
  GPIO_Pin pin;
} ADC_InitParams;

void ADC_initParamsInit(ADC_InitParams *initParams);
void ADC_init(ADC_InitParams *initParams);
ADC_Channel ADC_getChannelFromPortAndPin(GPIO_Port port, GPIO_Pin pin);

#endif
