
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

  RCC_peripheralClockEnableForPort(initParams->mosiPort);
  RCC_peripheralClockEnableForPort(initParams->misoPort);
  RCC_peripheralClockEnableForPort(initParams->sckPort);

  // GPIO AFIO
  if (initParams->halSpiInitParams.instance == SPI1
      && initParams->mosiPort == GPIOA && initParams->mosiPin == GPIO_Pin_7
      && initParams->misoPort == GPIOA && initParams->misoPin == GPIO_Pin_6
      && initParams->sckPort == GPIOA && initParams->sckPin == GPIO_Pin_5) {
    GPIO_setAlternateFunction(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, 0);
  } else {
    assert_param(0);
  }

  GPIO_initParamsInit(&gpio);
  gpio.port = initParams->mosiPort;
  gpio.pin = initParams->mosiPin;
  gpio.mode = GPIO_Mode_alternateFunction;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  gpio.port = initParams->sckPort;
  gpio.pin = initParams->sckPin;
  GPIO_init(&gpio);

  gpio.port = initParams->misoPort;
  gpio.pin = initParams->misoPin;
  GPIO_init(&gpio);

  HAL_SPI_init(&initParams->halSpiInitParams);
}

uint8_t SPI_transfer(SPI_Instance instance, uint8_t d) {
  while (SPI_getFlagStatus(instance, SPI_Flag_RXNE) == SET) {
    SPI_receiveData16(instance);
  }
  while (SPI_getFlagStatus(instance, SPI_Flag_TXE) == RESET);
  SPI_sendData8(instance, d);
  while (SPI_getFlagStatus(instance, SPI_Flag_RXNE) == RESET);
  return SPI_receiveData8(instance);
}


