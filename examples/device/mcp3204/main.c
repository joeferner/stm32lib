
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
  spiInit.halSpiInitParams.instance = MCP3204_SPI;
  spiInit.halSpiInitParams.cpol = SPI_CPOL_low;
  spiInit.halSpiInitParams.cpha = SPI_CPHA_1Edge;
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

  SPI_enable(MCP3204_SPI);
  
  printf("setup complete!\n");
}

static void loop() {
  uint16_t ch0, ch1, ch2, ch3;

  printf("begin sampling... ");
  ch0 = mcp3204_read(&mcp3204, MCP3204_ch0Single);
  ch1 = mcp3204_read(&mcp3204, MCP3204_ch1Single);
  ch2 = mcp3204_read(&mcp3204, MCP3204_ch2Single);
  ch3 = mcp3204_read(&mcp3204, MCP3204_ch3Single);
  printf(" ch0: %u, ch1: %u, ch2: %u, ch3: %u\n", ch0, ch1, ch2, ch3);

  sleep_ms(1000);
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}
