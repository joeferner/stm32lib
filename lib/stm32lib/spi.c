
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
  initParams->csPort = NULL;
  initParams->csPin = -1;
}

void SPI_init(SPI_InitParams *initParams) {
  GPIO_InitParams gpio;

  RCC_peripheralClockEnableForPort(initParams->mosiPort);
  RCC_peripheralClockEnableForPort(initParams->misoPort);
  RCC_peripheralClockEnableForPort(initParams->sckPort);
  if(initParams->csPort != NULL && initParams->csPin != -1) {
    RCC_peripheralClockEnableForPort(initParams->csPort);
  }
  if(initParams->halSpiInitParams.instance == SPI1) {
    RCC_peripheralClockEnable(RCC_Peripheral_SPI1);
  } else if(initParams->halSpiInitParams.instance == SPI2) {
    RCC_peripheralClockEnable(RCC_Peripheral_SPI2);
  } 
#ifdef SPI3
  else if(initParams->halSpiInitParams.instance == SPI3) {
    RCC_peripheralClockEnable(RCC_Peripheral_SPI3);
  }
#endif

  // GPIO AFIO
  if (initParams->halSpiInitParams.instance == SPI1
      && initParams->mosiPort == GPIOA && initParams->mosiPin == GPIO_Pin_7
      && initParams->misoPort == GPIOA && initParams->misoPin == GPIO_Pin_6
      && initParams->sckPort == GPIOA && initParams->sckPin == GPIO_Pin_5) {
    RCC_peripheralClockEnableForPort(GPIOA);
    GPIO_setAlternateFunction(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, 0);
  } else if (initParams->halSpiInitParams.instance == SPI2
      && initParams->mosiPort == GPIOB && initParams->mosiPin == GPIO_Pin_15
      && initParams->misoPort == GPIOB && initParams->misoPin == GPIO_Pin_14
      && initParams->sckPort == GPIOB && initParams->sckPin == GPIO_Pin_13) {
    RCC_peripheralClockEnableForPort(GPIOB);
    GPIO_setAlternateFunction(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, 0);
  }
#ifdef SPI3
  else if (initParams->halSpiInitParams.instance == SPI3
      && initParams->mosiPort == GPIOB && initParams->mosiPin == GPIO_Pin_5
      && initParams->misoPort == GPIOB && initParams->misoPin == GPIO_Pin_4
      && initParams->sckPort == GPIOB && initParams->sckPin == GPIO_Pin_3) {
    RCC_peripheralClockEnableForPort(GPIOB);
    GPIO_setAlternateFunction(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, 0);
  }
#endif
  else {
    assert_param(0);
  }

  GPIO_initParamsInit(&gpio);
  gpio.port = initParams->mosiPort;
  gpio.pin = initParams->mosiPin;
  gpio.mode = GPIO_Mode_alternateFunctionOutput;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  gpio.port = initParams->sckPort;
  gpio.pin = initParams->sckPin;
  gpio.mode = GPIO_Mode_alternateFunctionOutput;
  GPIO_init(&gpio);

  gpio.port = initParams->misoPort;
  gpio.pin = initParams->misoPin;
  gpio.mode = GPIO_Mode_alternateFunctionInput;
  GPIO_init(&gpio);

  if(initParams->csPort != NULL && initParams->csPin != -1) {
    gpio.port = initParams->csPort;
    gpio.pin = initParams->csPin;
    if(initParams->halSpiInitParams.mode == SPI_Mode_slave) {
      gpio.mode = GPIO_Mode_alternateFunctionInput;
    } else if(initParams->halSpiInitParams.mode == SPI_Mode_master) {
      gpio.mode = GPIO_Mode_alternateFunctionOutput;
    } else {
      assert_param(0);
    }
    GPIO_init(&gpio);
  }

  HAL_SPI_init(&initParams->halSpiInitParams);
}

uint8_t SPI_transfer(SPI_Instance instance, uint8_t d) {
  while (SPI_getFlagStatus(instance, SPI_Flag_RXNE) == SET) {
    SPI_receiveData16(instance);
  }
  SPI_waitForTxEmpty(instance);
  SPI_sendData8(instance, d);
  SPI_waitForRxNotEmpty(instance);
  return SPI_receiveData8(instance);
}

void SPI_waitForTxEmpty(SPI_Instance instance) {
  // If you are stuck here did you forget to call SPI_enable
  while (SPI_getFlagStatus(instance, SPI_Flag_TXE) == RESET);
}

void SPI_waitForRxNotEmpty(SPI_Instance instance) {
  while (SPI_getFlagStatus(instance, SPI_Flag_RXNE) == RESET);
}


