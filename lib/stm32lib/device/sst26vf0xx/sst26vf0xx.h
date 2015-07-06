#ifndef _sst26vf0xx_h_
#define _sst26vf0xx_h_

#include "../../hal/gpio.h"
#include "../../spi.h"

#define SST25VF0XX_SECTOR_SIZE    4096
#define SST26VF0XX_PAGE_SIZE      256
#define SST26VF0XX_UNIQUE_ID_SIZE 8

typedef struct {
  SPI_TypeDef *spi;
  GPIO_Port csPort;
  GPIO_Pin csPin;
} SST25VF0XX;

void sst26vf0xx_setup(SST25VF0XX *sst25vf0xx);
void sst26vf0xx_reset(SST25VF0XX *sst25vf0xx);
void sst26vf0xx_read(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t* buffer, uint16_t length);
void sst26vf0xx_write(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t* buffer, uint16_t length);
void sst26vf0xx_writeWithoutErase(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t* buffer, uint16_t length);
void sst26vf0xx_sectorErase(SST25VF0XX *sst25vf0xx, uint32_t address);
void sst26vf0xx_programPage(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t* buffer, uint16_t length);
void sst26vf0xx_readUniqueId(SST25VF0XX *sst25vf0xx, uint8_t* buffer);

#endif
