
#include "mcp3204.h"
#include "../../rcc.h"
#include "../../spi.h"

void _mcp3204_csDeassert(MCP3204 *mcp3204);
void _mcp3204_csAssert(MCP3204 *mcp3204);

void mcp3204_setup(MCP3204 *mcp3204) {
  GPIO_InitParams gpio;

  RCC_peripheralClockEnableForPort(mcp3204->csPort);

  GPIO_initParamsInit(&gpio);
  gpio.port = mcp3204->csPort;
  gpio.pin = mcp3204->csPin;
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);
  _mcp3204_csDeassert(mcp3204);
}

void _mcp3204_csDeassert(MCP3204 *mcp3204) {
  GPIO_setBits(mcp3204->csPort, mcp3204->csPin);
}

void _mcp3204_csAssert(MCP3204 *mcp3204) {
  GPIO_resetBits(mcp3204->csPort, mcp3204->csPin);
}

uint16_t mcp3204_read(MCP3204 *mcp3204, MCP3204_ch ch) {
  uint16_t high, low, value;
  
  assert_param(IS_MCP3204_CH(ch));

  _mcp3204_csAssert(mcp3204);
  SPI_transfer(mcp3204->spi, (ch >> 8) & 0xff);
  sleep_us(400);
  high = SPI_transfer(mcp3204->spi, ch & 0xff) & 0x0f;
  sleep_us(400);
  low = SPI_transfer(mcp3204->spi, 0x00);
  sleep_us(400);
  value = (high << 8) | low;
  _mcp3204_csDeassert(mcp3204);
  return value;
}
