
#include "mcp3204.h"
#include "../../rcc.h"

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
  _mcp3204_csAssert(mcp3204);
  // TODO
  _mcp3204_csDeassert(mcp3204);
}
