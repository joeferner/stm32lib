
#include <string.h>
#include <stdio.h>
#include "sst26vf0xx.h"
#include "../../rcc.h"
#include "../../hal/gpio.h"
#include "../../time.h"
#include "../../utils.h"
#include "platform_config.h"

#ifdef SST26VF0XX_DEBUG
#  define debug_printf(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#  define debug_printf(fmt, ...)  
#endif

typedef enum {
  _SST26VF0XX_COMMAND_PAGE_PROGRAM     = 0x02,
  _SST26VF0XX_COMMAND_READ             = 0x03,
  _SST26VF0XX_COMMAND_WRITE_DISABLE    = 0x04,
  _SST26VF0XX_COMMAND_READ_STATUS      = 0x05,
  _SST26VF0XX_COMMAND_WRITE_ENABLE     = 0x06,
  _SST26VF0XX_COMMAND_SECTOR_ERASE     = 0x20,
  _SST26VF0XX_COMMAND_READ_CONFIG      = 0x35,
  _SST26VF0XX_COMMAND_RSTEN            = 0x66,
  _SST26VF0XX_COMMAND_READ_SECURITY_ID = 0x88,
  _SST26VF0XX_COMMAND_GLOBAL_UNLOCK    = 0x98,
  _SST26VF0XX_COMMAND_RESET            = 0x99
} _SST26VF0XX_COMMANDS;

#define _SST26VF0XX_STATUS_BUSY 0x01
#define _SST26VF0XX_STATUS_WEL  0x02
#define _SST26VF0XX_STATUS_WSE  0x04
#define _SST26VF0XX_STATUS_WSP  0x08
#define _SST26VF0XX_STATUS_WPLD 0x10
#define _SST26VF0XX_STATUS_SEC  0x20

#define _SST26VF0XX_CONFIG_IOC  0x02
#define _SST26VF0XX_CONFIG_BPNV 0x08
#define _SST26VF0XX_CONFIG_WPEN 0x80

void _sst26vf0xx_csDeassert(SST25VF0XX *sst25vf0xx);
void _sst26vf0xx_csAssert(SST25VF0XX *sst25vf0xx);
void _sst26vf0xx_transferCommandAndAddress(SST25VF0XX *sst25vf0xx, _SST26VF0XX_COMMANDS command, uint32_t address);
void _sst26vf0xx_pollBusy(SST25VF0XX *sst25vf0xx);
uint8_t _sst26vf0xx_readStatusRegister(SST25VF0XX *sst25vf0xx);
uint8_t _sst26vf0xx_readConfigurationRegister(SST25VF0XX *sst25vf0xx);
void _sst26vf0xx_writeEnable(SST25VF0XX *sst25vf0xx);
void _sst26vf0xx_writeDisable(SST25VF0XX *sst25vf0xx);
void _sst26vf0xx_globalBlockProtectionUnlock(SST25VF0XX *sst25vf0xx);

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
  _sst26vf0xx_globalBlockProtectionUnlock(sst25vf0xx);

  _sst26vf0xx_readStatusRegister(sst25vf0xx);
  _sst26vf0xx_readConfigurationRegister(sst25vf0xx);
  uint8_t uniqueIdBuffer[SST26VF0XX_UNIQUE_ID_SIZE];
  sst26vf0xx_readUniqueId(sst25vf0xx, uniqueIdBuffer);
}

void _sst26vf0xx_csDeassert(SST25VF0XX *sst25vf0xx) {
  GPIO_setBits(sst25vf0xx->csPort, sst25vf0xx->csPin);
}

void _sst26vf0xx_csAssert(SST25VF0XX *sst25vf0xx) {
  GPIO_resetBits(sst25vf0xx->csPort, sst25vf0xx->csPin);
}

void sst26vf0xx_reset(SST25VF0XX *sst25vf0xx) {
  debug_printf("sst26vf0xx_reset\n");
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_RSTEN);
  _sst26vf0xx_csDeassert(sst25vf0xx);
  sleep_us(100);
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_RESET);
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

void sst26vf0xx_read(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t *buffer, uint16_t length) {
  debug_printf("sst26vf0xx_read: address: 0x%08lx, length: %d\n", address, length);
  _sst26vf0xx_csAssert(sst25vf0xx);
  _sst26vf0xx_transferCommandAndAddress(sst25vf0xx, _SST26VF0XX_COMMAND_READ, address);
  uint8_t *p = buffer;
  for (int i = 0; i < length; i++) {
    *p++ = SPI_transfer(sst25vf0xx->spi, 0x00);
  }
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

void sst26vf0xx_write(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t *buffer, uint16_t length) {
  uint8_t writeBuffer[SST25VF0XX_SECTOR_SIZE];
  uint32_t sectorOffset = address % SST25VF0XX_SECTOR_SIZE;
  uint32_t sectorStart = address - (address % SST25VF0XX_SECTOR_SIZE);
  int32_t bytesToWrite = length;
  int32_t bytesFromBuffer;
  uint8_t *p = buffer;

  debug_printf("sst26vf0xx_write: address: 0x%08lx, length: %d\n", address, length);
  while (bytesToWrite > 0) {
    sst26vf0xx_read(sst25vf0xx, sectorStart, writeBuffer, SST25VF0XX_SECTOR_SIZE);
    bytesFromBuffer = min(length, SST25VF0XX_SECTOR_SIZE - sectorOffset);
    memcpy(writeBuffer, p, bytesFromBuffer);
    sst26vf0xx_sectorErase(sst25vf0xx, sectorStart);
    sst26vf0xx_writeWithoutErase(sst25vf0xx, sectorStart, writeBuffer, SST25VF0XX_SECTOR_SIZE);
    sectorStart += SST25VF0XX_SECTOR_SIZE;
    bytesToWrite -= SST25VF0XX_SECTOR_SIZE;
    p += bytesFromBuffer;
    sectorOffset = 0;
  }
}

void sst26vf0xx_writeWithoutErase(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t *buffer, uint16_t length) {
  assert_param((address % SST26VF0XX_PAGE_SIZE) == 0);
  debug_printf("sst26vf0xx_writeWithoutErase: address: 0x%08lx, length: %d\n", address, length);
  uint16_t bytesToWrite = length;
  uint32_t writeAddress = address;
  uint16_t bytesToWriteInPage;
  uint8_t *p = buffer;

  while (bytesToWrite > 0) {
    bytesToWriteInPage = min(SST26VF0XX_PAGE_SIZE, bytesToWrite);
    sst26vf0xx_programPage(sst25vf0xx, writeAddress, p, bytesToWriteInPage);
    bytesToWrite -= bytesToWriteInPage;
    p += bytesToWriteInPage;
  }
}

void sst26vf0xx_programPage(SST25VF0XX *sst25vf0xx, uint32_t address, uint8_t *buffer, uint16_t length) {
  debug_printf("_sst26vf0xx_programPage: address: 0x%08lx, length: %d\n", address, length);
  uint8_t *p = buffer;

  _sst26vf0xx_writeEnable(sst25vf0xx);
  _sst26vf0xx_csAssert(sst25vf0xx);
  _sst26vf0xx_transferCommandAndAddress(sst25vf0xx, _SST26VF0XX_COMMAND_PAGE_PROGRAM, address);
  for (int i = 0; i < length; i++) {
    SPI_transfer(sst25vf0xx->spi, *p++);
  }
  _sst26vf0xx_csDeassert(sst25vf0xx);
  _sst26vf0xx_pollBusy(sst25vf0xx);
  _sst26vf0xx_writeDisable(sst25vf0xx);
}

void sst26vf0xx_sectorErase(SST25VF0XX *sst25vf0xx, uint32_t address) {
  debug_printf("_sst26vf0xx_sectorErase: address: 0x%08lx\n", address);
  _sst26vf0xx_writeEnable(sst25vf0xx);
  _sst26vf0xx_csAssert(sst25vf0xx);
  _sst26vf0xx_transferCommandAndAddress(sst25vf0xx, _SST26VF0XX_COMMAND_SECTOR_ERASE, address);
  _sst26vf0xx_csDeassert(sst25vf0xx);
  _sst26vf0xx_pollBusy(sst25vf0xx);
  _sst26vf0xx_writeDisable(sst25vf0xx);
}

void _sst26vf0xx_transferCommandAndAddress(SST25VF0XX *sst25vf0xx, _SST26VF0XX_COMMANDS command, uint32_t address) {
  SPI_transfer(sst25vf0xx->spi, command);
  SPI_transfer(sst25vf0xx->spi, (address >> 16) & 0xff);
  SPI_transfer(sst25vf0xx->spi, (address >> 8) & 0xff);
  SPI_transfer(sst25vf0xx->spi, (address >> 0) & 0xff);
}

void _sst26vf0xx_pollBusy(SST25VF0XX *sst25vf0xx) {
  while (_sst26vf0xx_readStatusRegister(sst25vf0xx) & _SST26VF0XX_STATUS_BUSY)
    ;
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

uint8_t _sst26vf0xx_readStatusRegister(SST25VF0XX *sst25vf0xx) {
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_READ_STATUS);
  uint8_t status = SPI_transfer(sst25vf0xx->spi, 0x00);
  _sst26vf0xx_csDeassert(sst25vf0xx);
  debug_printf("_sst26vf0xx_readStatusRegister: 0x%02x\n", status);
  return status;
}

uint8_t _sst26vf0xx_readConfigurationRegister(SST25VF0XX *sst25vf0xx) {
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_READ_CONFIG);
  uint8_t status = SPI_transfer(sst25vf0xx->spi, 0x00);
  _sst26vf0xx_csDeassert(sst25vf0xx);
  debug_printf("_sst26vf0xx_readConfigurationRegister: 0x%02x\n", status);
  return status;
}

void _sst26vf0xx_writeEnable(SST25VF0XX *sst25vf0xx) {
  debug_printf("_sst26vf0xx_writeEnable\n");
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_WRITE_ENABLE);
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

void _sst26vf0xx_writeDisable(SST25VF0XX *sst25vf0xx) {
  debug_printf("_sst26vf0xx_writeDisable\n");
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_WRITE_DISABLE);
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

void sst26vf0xx_readUniqueId(SST25VF0XX *sst25vf0xx, uint8_t *buffer) {
  uint8_t *p = buffer;
  debug_printf("sst26vf0xx_readUniqueId: ");
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_READ_SECURITY_ID);
  SPI_transfer(sst25vf0xx->spi, 0x00);
  SPI_transfer(sst25vf0xx->spi, 0x00);
  SPI_transfer(sst25vf0xx->spi, 0x00); // dummy
  for (uint8_t i = 0; i < SST26VF0XX_UNIQUE_ID_SIZE; i++) {
    *p = SPI_transfer(sst25vf0xx->spi, 0x00);
    debug_printf("%02x", *p);
    p++;
  }
  _sst26vf0xx_csDeassert(sst25vf0xx);
  debug_printf("\n");
}

void _sst26vf0xx_globalBlockProtectionUnlock(SST25VF0XX *sst25vf0xx) {
  debug_printf("_sst26vf0xx_globalBlockProtectionUnlock\n");
  _sst26vf0xx_writeEnable(sst25vf0xx);
  _sst26vf0xx_csAssert(sst25vf0xx);
  SPI_transfer(sst25vf0xx->spi, _SST26VF0XX_COMMAND_GLOBAL_UNLOCK);
  _sst26vf0xx_csDeassert(sst25vf0xx);
}

