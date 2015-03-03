
#include "sdcard.h"
#include "time.h"
#include <stdio.h>
#include "info.h"
#include "../../time.h"

#define SDCARD_BLOCK_SIZE 512

/** Standard capacity V1 SD card */
uint8_t const SD_CARD_TYPE_SD1 = 1;
/** Standard capacity V2 SD card */
uint8_t const SD_CARD_TYPE_SD2 = 2;
/** High Capacity SD card */
uint8_t const SD_CARD_TYPE_SDHC = 3;

/** init timeout ms */
uint16_t const SD_INIT_TIMEOUT = 2000;
/** erase timeout ms */
uint16_t const SD_ERASE_TIMEOUT = 10000;
/** read timeout ms */
uint16_t const SD_READ_TIMEOUT = 300;
/** write time out ms */
uint16_t const SD_WRITE_TIMEOUT = 600;

void _SDCard_cs_deassert(SDCard_InitParams *initParams);
void _SDCard_cs_assert(SDCard_InitParams *initParams);
uint8_t _SDCard_spiTransfer(SDCard_InitParams *initParams, uint8_t d);
uint8_t _SDCard_command(SDCard_InitParams *initParams, uint8_t cmd, uint32_t arg);
uint8_t _SDCard_acommand(SDCard_InitParams *initParams, uint8_t acmd, uint32_t arg);
uint8_t _SDCard_waitStartBlock(SDCard_InitParams *initParams);
uint8_t _SDCard_waitNotBusy(SDCard_InitParams *initParams, uint16_t timeoutMillis);

void SDCard_initParamsInit(SDCard_InitParams *initParams) {
  initParams->spiInstance = NULL;
  initParams->csPort = NULL;
  initParams->csPin = -1;
  initParams->cardType = -1;
}

bool SDCard_init(SDCard_InitParams *initParams) {
  uint16_t t0 = (uint16_t)time_ms();
  uint32_t arg;
  uint8_t status;
  GPIO_InitParams gpio;

  GPIO_initParamsInit(&gpio);
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.pin = initParams->csPin;
  gpio.port = initParams->csPort;
  gpio.pullUpDown = GPIO_PullUpDown_pullUp;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);
  _SDCard_cs_deassert(initParams);

  RCC_TypeDef *zrcc = RCC;
  SPI_TypeDef *zspi = SPI2;
  GPIO_TypeDef *zgpio = GPIOB;
  
  // must supply min of 74 clock cycles with CS high.
  for (uint8_t i = 0; i < 10; i++) {
    _SDCard_spiTransfer(initParams, 0XFF);
  }

  _SDCard_cs_assert(initParams);

  // command to go idle in SPI mode
  while ((status = _SDCard_command(initParams, CMD0, 0)) != R1_IDLE_STATE) {
    if (((uint16_t)time_ms() - t0) > SD_INIT_TIMEOUT) {
      printf("SD_CARD_ERROR_CMD0\n");
      goto fail;
    }
  }

  // check SD version
  if ((_SDCard_command(initParams, CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
    initParams->cardType = SD_CARD_TYPE_SD1;
  } else {
    // only need last byte of r7 response
    for (uint8_t i = 0; i < 4; i++) {
      status = _SDCard_spiTransfer(initParams, 0xff);
    }
    if (status != 0XAA) {
      printf("SD_CARD_ERROR_CMD8\n");
      goto fail;
    }
    initParams->cardType = SD_CARD_TYPE_SD2;
  }

  // initialize card and send host supports SDHC if SD2
  arg = initParams->cardType == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

  while ((status = _SDCard_acommand(initParams, ACMD41, arg)) != R1_READY_STATE) {
    // check for timeout
    if (((uint16_t)time_ms() - t0) > SD_INIT_TIMEOUT) {
      printf("SD_CARD_ERROR_ACMD41\n");
      goto fail;
    }
  }

  // if SD2 read OCR register to check for SDHC card
  if (initParams->cardType == SD_CARD_TYPE_SD2) {
    if (_SDCard_command(initParams, CMD58, 0)) {
      printf("SD_CARD_ERROR_CMD58\n");
      goto fail;
    }
    if ((_SDCard_spiTransfer(initParams, 0xff) & 0XC0) == 0XC0) {
      initParams->cardType = SD_CARD_TYPE_SDHC;
    }
    // discard rest of ocr - contains allowed voltage range
    for (uint8_t i = 0; i < 3; i++) {
      _SDCard_spiTransfer(initParams, 0xff);
    }
  }

  _SDCard_cs_deassert(initParams);
  printf("END SDCard Setup (type: %u)\n", initParams->cardType);

  return true;

fail:
  _SDCard_cs_deassert(initParams);
  return false;
}

void _SDCard_cs_deassert(SDCard_InitParams *initParams) {
  GPIO_setBits(initParams->csPort, initParams->csPin);
}

void _SDCard_cs_assert(SDCard_InitParams *initParams) {
  GPIO_resetBits(initParams->csPort, initParams->csPin);
}

bool SDCard_readBlock(SDCard_InitParams *initParams, uint32_t block, uint8_t *data) {
  uint16_t n;

  // use address if not SDHC card
  if (initParams->cardType != SD_CARD_TYPE_SDHC) {
    block <<= 9;
  }

  if (_SDCard_command(initParams, CMD17, block)) {
    printf("SD_CARD_ERROR_CMD17\n");
    goto fail;
  }

  if (!_SDCard_waitStartBlock(initParams)) {
    goto fail;
  }

  for (n = 0; n < SDCARD_BLOCK_SIZE; n++) {
    data[n] = _SDCard_spiTransfer(initParams, 0xff);
  }

  _SDCard_cs_deassert(initParams);
  return true;

fail:
  _SDCard_cs_deassert(initParams);
  return false;
}

bool SDCard_writeBlock(SDCard_InitParams *initParams, uint32_t blockNumber, const uint8_t *data) {
  int16_t crc = 0xffff; // Dummy CRC value
  int i;

  // don't allow write to first block
  if (blockNumber == 0) {
    printf("SD_CARD_ERROR_WRITE_BLOCK_ZERO\n");
    goto fail;
  }

  // use address if not SDHC card
  if (initParams->cardType != SD_CARD_TYPE_SDHC) {
    blockNumber <<= 9;
  }

  if (_SDCard_command(initParams, CMD24, blockNumber)) {
    printf("SD_CARD_ERROR_CMD24\n");
    goto fail;
  }

  _SDCard_spiTransfer(initParams, DATA_START_BLOCK);
  for (i = 0; i < SDCARD_BLOCK_SIZE; i++) {
    _SDCard_spiTransfer(initParams, data[i]);
  }

  _SDCard_spiTransfer(initParams, crc >> 8);
  _SDCard_spiTransfer(initParams, crc);

  if ((_SDCard_spiTransfer(initParams, 0xff) & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    printf("SD_CARD_ERROR_WRITE\n");
    goto fail;
  }

  // wait for flash programming to complete
  if (!_SDCard_waitNotBusy(initParams, SD_WRITE_TIMEOUT)) {
    printf("SD_CARD_ERROR_WRITE_TIMEOUT\n");
    goto fail;
  }

  // response is r2 so get and check two bytes for nonzero
  if (_SDCard_command(initParams, CMD13, 0) || _SDCard_spiTransfer(initParams, 0xff)) {
    printf("SD_CARD_ERROR_WRITE_PROGRAMMING\n");
    goto fail;
  }

  _SDCard_cs_deassert(initParams);
  return true;

fail:
  _SDCard_cs_deassert(initParams);
  return false;
}

uint8_t _SDCard_command(SDCard_InitParams *initParams, uint8_t cmd, uint32_t arg) {
  uint8_t status;

  _SDCard_cs_assert(initParams);

  _SDCard_waitNotBusy(initParams, SD_READ_TIMEOUT);

  // send command
  _SDCard_spiTransfer(initParams, cmd | 0x40);

  // send argument
  for (int8_t s = 24; s >= 0; s -= 8) {
    _SDCard_spiTransfer(initParams, arg >> s);
  }

  // send CRC
  uint8_t crc = 0XFF;
  if (cmd == CMD0) {
    crc = 0X95; // correct crc for CMD0 with arg 0
  }
  if (cmd == CMD8) {
    crc = 0X87; // correct crc for CMD8 with arg 0X1AA
  }
  _SDCard_spiTransfer(initParams, crc);

  // wait for response
  for (uint8_t i = 0; ((status = _SDCard_spiTransfer(initParams, 0xff)) & 0X80) && i != 0XFF; i++);
  return status;
}

uint8_t _SDCard_acommand(SDCard_InitParams *initParams, uint8_t acmd, uint32_t arg) {
  _SDCard_command(initParams, CMD55, 0);
  return _SDCard_command(initParams, acmd, arg);
}

uint8_t _SDCard_spiTransfer(SDCard_InitParams *initParams, uint8_t d) {
  return SPI_transfer(initParams->spiInstance, d);
}

uint8_t _SDCard_waitStartBlock(SDCard_InitParams *initParams) {
  uint8_t status;
  uint16_t t0 = time_ms();
  while ((status = _SDCard_spiTransfer(initParams, 0xff)) == 0XFF) {
    if (((uint16_t)time_ms() - t0) > SD_READ_TIMEOUT) {
      printf("SD_CARD_ERROR_READ_TIMEOUT\n");
      goto fail;
    }
  }
  if (status != DATA_START_BLOCK) {
    printf("SD_CARD_ERROR_READ\n");
    goto fail;
  }
  return true;

fail:
  _SDCard_cs_deassert(initParams);
  return false;
}

uint8_t _SDCard_waitNotBusy(SDCard_InitParams *initParams, uint16_t timeoutMillis) {
  uint16_t t0 = time_ms();
  do {
    if (_SDCard_spiTransfer(initParams, 0xff) == 0XFF) {
      return true;
    }
  } while (((uint16_t)time_ms() - t0) < timeoutMillis);
  return false;
}
