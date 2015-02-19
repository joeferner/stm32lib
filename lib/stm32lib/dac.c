
#include "dac.h"
#include "rcc.h"

void DAC_initParamsInit(DAC_InitParams *initParams) {
  HAL_DAC_initParamsInit(&initParams->halDacInitParams);
  initParams->port = NULL;
  initParams->pin = -1;
}

void DAC_init(DAC_InitParams *initParams) {
  GPIO_InitParams gpio;

  if (initParams->halDacInitParams.channel == -1) {
    initParams->halDacInitParams.channel = DAC_getChannelFromPortAndPin(initParams->port, initParams->pin);
  }

  RCC_peripheralClockEnable(RCC_peripheral_DAC);

  GPIO_initParamsInit(&gpio);
  gpio.port = initParams->port;
  gpio.pin = initParams->pin;
  gpio.mode = GPIO_Mode_analog;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  HAL_DAC_init(&initParams->halDacInitParams);
}

DAC_Channel DAC_getChannelFromPortAndPin(GPIO_Port port, GPIO_Pin pin) {
  if (port == GPIOA && pin == GPIO_Pin_4) {
    return DAC_Channel_1;
  }
  if (port == GPIOA && pin == GPIO_Pin_5) {
    return DAC_Channel_2;
  }
  assert_param(0);
  return -1;
}

