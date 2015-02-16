
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
} SPI_InitParams;

void SPI_initParamsInit(SPI_InitParams *initParams);
void SPI_init(SPI_InitParams *initParams);

#endif
