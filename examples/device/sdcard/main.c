
#include "platform_config.h"
#include <stdio.h>

SDCardFAT fat;

static void setup();
static void readFile();
static void spi_setup();
static void sdcard_setup();

int main(void) {
  setup();
  readFile();
  return 0;
}

static void setup() {
  debug_setup();
  spi_setup();
  sdcard_setup();
  printf("setup complete!\n");
}

static void spi_setup() {
  SPI_InitParams spiInit;
  
  printf("begin spi_setup\n");
  SPI_initParamsInit(&spiInit);
  spiInit.halSpiInitParams.instance = SDCARD_SPI;
  spiInit.halSpiInitParams.cpol = SDCARD_SPI_CPOL;
  spiInit.halSpiInitParams.cpha = SDCARD_SPI_CPHA;
  spiInit.sckPort = SDCARD_SCK_PORT;
  spiInit.sckPin = SDCARD_SCK_PIN;
  spiInit.misoPort = SDCARD_MISO_PORT;
  spiInit.misoPin = SDCARD_MISO_PIN;
  spiInit.mosiPort = SDCARD_MOSI_PORT;
  spiInit.mosiPin = SDCARD_MOSI_PIN;
  SPI_init(&spiInit);
  SPI_enable(SDCARD_SPI);
  printf("spi_setup complete\n");
}

static void sdcard_setup() {
  printf("begin sdcard setup\n");
  SDCardFAT_initParamsInit(&fat);
  fat.sdcard.spiInstance = SDCARD_SPI;
  fat.sdcard.csPort = SDCARD_CS_PORT;
  fat.sdcard.csPin = SDCARD_CS_PIN;
  SDCardFAT_init(&fat);
  printf("setup sdcard complete\n");
}

static void readFile() {
  SDCardFATFile f;
  uint8_t buffer[100];
  int read;
  
  printf("begin read file\n");
  
  if(!SDCardFATFile_open(&fat, &f, "readme.txt", O_RDONLY)) {
    printf("failed to open file\n");
    return;
  }
  
  printf("readme.txt contents:\n");
  while((read = SDCardFATFile_read(&fat, &f, buffer, sizeof(buffer))) > 0) {
    printf("%s", (const char*)buffer);
  }
  printf("\n");
  
  SDCardFATFile_close(&fat, &f);
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}
