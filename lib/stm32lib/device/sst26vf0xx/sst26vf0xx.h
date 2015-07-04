#ifndef _sst26vf0xx_h_
#define _sst26vf0xx_h_

#include "../../hal/gpio.h"
#include "../../spi.h"

typedef struct {
  SPI_TypeDef *spi;
  GPIO_Port csPort;
  GPIO_Pin csPin;
} SST25VF0XX;

void sst26vf0xx_setup(SST25VF0XX *sst25vf0xx);
void sst26vf0xx_reset(SST25VF0XX *sst25vf0xx);
void sst26vf0xx_read(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t* buffer, uint16_t length);

#endif
