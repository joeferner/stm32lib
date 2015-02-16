
#include "spi.h"
#include "rcc.h"

void SPI_initParamsInit(SPI_InitParams *initParams) {
  HAL_SPI_initParamsInit(&initParams->halSpiInitParams);
  initParams->mosiPort = NULL;
  initParams->mosiPin = -1;
  initParams->misoPort = NULL;
  initParams->misoPin = -1;
  initParams->sckPort = NULL;
  initParams->sckPin = -1;
}

void SPI_init(SPI_InitParams *initParams) {
  GPIO_InitParams gpio;

  HAL_SPI_init(&initParams->halSpiInitParams);

  RCC_peripheralClockEnableForPort(initParams->mosiPort);
  RCC_peripheralClockEnableForPort(initParams->misoPort);
  RCC_peripheralClockEnableForPort(initParams->sckPort);

  GPIO_initParamsInit(&gpio);
  gpio.port = initParams->mosiPort;
  gpio.pin = initParams->mosiPin;
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  gpio.port = initParams->sckPort;
  gpio.pin = initParams->sckPin;
  GPIO_init(&gpio);

  gpio.port = initParams->misoPort;
  gpio.pin = initParams->misoPin;
  gpio.mode = GPIO_Mode_input;
  GPIO_init(&gpio);
}

