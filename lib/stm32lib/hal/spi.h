
#ifndef _STM32LIB_HAL_SPI_H_
#define _STM32LIB_HAL_SPI_H_

typedef enum {
  SPI_Direction_2LinesFullDuplex
} SPI_Direction;

typedef enum {
  SPI_Mode_master
} SPI_Mode;

typedef enum {
  SPI_DataSize_8b
} SPI_DataSize;

typedef enum {
  SPI_NSS_Soft
} SPI_NSS;

typedef enum {
  SPI_BaudRatePrescaler_1,
  SPI_BaudRatePrescaler_16
} SPI_BaudRatePrescaler;

typedef enum {
  SPI_FirstBit_msb
} SPI_FirstBit;

typedef struct {
  SPI_TypeDef *spi;
  SPI_Direction direction;
  SPI_Mode mode;
  SPI_DataSize dataSize;
  SPI_NSS nss;
  SPI_BaudRatePrescaler baudRatePrescaler;
  SPI_FirstBit firstBit;
} HAL_SPI_InitParams;

void HAL_SPI_initParamsInit(HAL_SPI_InitParams *initParams);
void HAL_SPI_init(HAL_SPI_InitParams *initParams);

#endif
