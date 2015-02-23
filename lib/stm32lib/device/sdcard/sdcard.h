
#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <stdint.h>
#include <stdbool.h>
#include "../../hal/gpio.h"
#include "../../spi.h"

typedef struct {
  bool initializeSpi;
  SPI_Instance spiInstance;
  GPIO_Port csPort;
  GPIO_Pin csPin;
  uint8_t cardType;
} SDCard_InitParams;

void SDCard_initParamsInit(SDCard_InitParams* initParams);
bool SDCard_init(SDCard_InitParams* initParams);
bool SDCard_readBlock(SDCard_InitParams *initParams, uint32_t block, uint8_t *data);
bool SDCard_writeBlock(SDCard_InitParams *initParams, uint32_t blockNumber, const uint8_t *data);

#endif
