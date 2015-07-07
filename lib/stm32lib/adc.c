
#include "adc.h"
#include "rcc.h"

void ADC_initParamsInit(ADC_InitParams *initParams) {
  HAL_ADC_initParamsInit(&initParams->halAdcInitParams);
  initParams->port = NULL;
  initParams->pin = -1;
}

void ADC_init(ADC_InitParams *initParams) {
  GPIO_InitParams gpio;

  if (initParams->halAdcInitParams.channel == ADC_Channel_notSet) {
    initParams->halAdcInitParams.channel = ADC_getChannelFromPortAndPin(initParams->port, initParams->pin);
  }

  if(initParams->halAdcInitParams.instance == ADC1) {
    RCC_peripheralClockEnable(RCC_Peripheral_ADC1);
  } else {
    assert_param(0);
  }

  GPIO_initParamsInit(&gpio);
  gpio.port = initParams->port;
  gpio.pin = initParams->pin;
  gpio.mode = GPIO_Mode_input;
  gpio.speed = GPIO_Speed_low;
  GPIO_init(&gpio);

  HAL_ADC_init(&initParams->halAdcInitParams);
}

ADC_Channel ADC_getChannelFromPortAndPin(GPIO_Port port, GPIO_Pin pin) {
  if (port == GPIOA && pin == GPIO_Pin_4) {
    return ADC_Channel_4;
  }
  assert_param(0);
  return -1;
}
