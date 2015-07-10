
#ifndef _STM32LIB_SPI_H_
#define _STM32LIB_SPI_H_

#include "hal/spi.h"
#include "hal/gpio.h"

typedef struct {
  HAL_SPI_InitParams halSpiInitParams;
  GPIO_Port mosiPort;
  GPIO_Pin mosiPin;
  GPIO_Port misoPort;
  GPIO_Pin misoPin;
  GPIO_Port sckPort;
  GPIO_Pin sckPin;
  GPIO_Port csPort;
  GPIO_Pin csPin;
} SPI_InitParams;

void SPI_initParamsInit(SPI_InitParams *initParams);
void SPI_init(SPI_InitParams *initParams);
uint8_t SPI_transfer(SPI_Instance instance, uint8_t d);
void SPI_waitForTxEmpty(SPI_Instance instance);
void SPI_waitForRxNotEmpty(SPI_Instance instance);

#endif
