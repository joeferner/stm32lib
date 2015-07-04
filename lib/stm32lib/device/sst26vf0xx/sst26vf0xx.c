
#include "sst26vf0xx.h"
#include "../../rcc.h"
#include "../../hal/gpio.h"
#include "../../time.h"

#define _SST26VF0XX_COMMAND_RSTEN  0x66
#define _SST26VF0XX_COMMAND_RESET  0x99
#define _SST26VF0XX_COMMAND_READ   0x03

void _sst26vf0xx_csDeassert(SST25VF0XX *sst25vf0xx);
void _sst26vf0xx_csAssert(SST25VF0XX *sst25vf0xx);

void sst26vf0xx_setup(SST25VF0XX *sst25vf0xx) {
  GPIO_InitParams gpio;

  RCC_peripheralClockEnableForPort(sst25vf0xx->csPort);

  GPIO_initParamsInit(&gpio);
  gpio.port = sst25vf0xx->csPort;
  gpio.pin = sst25vf0xx->csPin;
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);
  _sst26vf0xx_csDeassert(sst25vf0xx);
  
  sleep_ms(1);
  sst26vf0xx_reset(sst25vf0xx);
}

void _sst26vf0xx_csDeassert(SST25VF0XX *sst25vf0xx) {
  GPIO_setBits(sst25vf0xx->csPort, sst25vf0xx->csPin);
}

void _sst26vf0xx_csAssert(SST25VF0XX *sst25vf0xx) {
  GPIO_resetBits(sst25vf0xx->csPort, sst25vf0xx->csPin);
}

void sst26vf0xx_reset(SST25VF0XX *sst25vf0xx) {
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_RSTEN);
  _sst26vf0xx_csDeassert(sst25vf0xx);
  sleep_us(100);
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_RESET);
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

void sst26vf0xx_read(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t* buffer, uint16_t length) {
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_READ);
  SPI_transfer(sst25vf0xx->spi, (address >> 16) & 0xff);
  SPI_transfer(sst25vf0xx->spi, (address >> 8) & 0xff);
  SPI_transfer(sst25vf0xx->spi, (address >> 0) & 0xff);
  uint8_t *p = buffer;
  for(int i = 0; i < length; i++) {
    *p++ = SPI_transfer(sst25vf0xx->spi, 0x00);
  }
  _sst26vf0xx_csDeassert(sst25vf0xx);
}


