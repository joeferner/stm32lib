
#include "spi.h"

void HAL_SPI_initParamsInit(HAL_SPI_InitParams* initParams) {
  initParams->spi = NULL;
  initParams->direction = SPI_Direction_2LinesFullDuplex;
  initParams->mode = SPI_Mode_master;
  initParams->dataSize = SPI_DataSize_8b;
  initParams->nss = SPI_NSS_Soft;
  initParams->baudRatePrescaler = SPI_BaudRatePrescaler_1;
  initParams->firstBit = SPI_FirstBit_msb;  
}

void HAL_SPI_init(HAL_SPI_InitParams* initParams) {
  // TODO
}

