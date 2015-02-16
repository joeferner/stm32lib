
#include "platform_config.h"
#include <stdio.h>

MCP3204 mcp3204;

static void setup();
static void loop();

int main(void) {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

static void setup() {
  SPI_InitParams spiInit;

  debug_setup();

  SPI_initParamsInit(&spiInit);
  spiInit.halSpiInitParams.spi = MCP3204_SPI;
  spiInit.mosiPort = MCP3204_MOSI_PORT;
  spiInit.mosiPin = MCP3204_MOSI_PIN;
  spiInit.misoPort = MCP3204_MISO_PORT;
  spiInit.misoPin = MCP3204_MISO_PIN;
  spiInit.sckPort = MCP3204_SCK_PORT;
  spiInit.sckPin = MCP3204_SCK_PIN;
  SPI_init(&spiInit);

  mcp3204.spi = MCP3204_SPI;
  mcp3204.csPort = MCP3204_CS_PORT;
  mcp3204.csPin = MCP3204_CS_PIN;
  mcp3204_setup(&mcp3204);

  printf("setup complete!\n");
}

static void loop() {

}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}
