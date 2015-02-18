
#ifndef _STM32LIB_DEVICE_MCP3204_H_
#define _STM32LIB_DEVICE_MCP3204_H_

#include <stdint.h>
#include "../../hal/gpio.h"

typedef enum {
  MCP3204_ch0Single = 0x0600,
  MCP3204_ch1Single = 0x0640,
  MCP3204_ch2Single = 0x0680,
  MCP3204_ch3Single = 0x06c0
} MCP3204_ch;
#define IS_MCP3204_CH(ch) ( \
  ((ch) == MCP3204_ch0Single) \
  || ((ch) == MCP3204_ch1Single) \
  || ((ch) == MCP3204_ch2Single) \
  || ((ch) == MCP3204_ch3Single) \
)

typedef struct {
  SPI_TypeDef *spi;
  GPIO_Port csPort;
  GPIO_Pin csPin;
} MCP3204;

void mcp3204_setup(MCP3204 *mcp3204);
uint16_t mcp3204_read(MCP3204 *mcp3204, MCP3204_ch ch);

#endif
