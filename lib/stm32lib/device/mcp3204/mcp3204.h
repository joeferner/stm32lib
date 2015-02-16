
#ifndef _STM32LIB_DEVICE_MCP3204_H_
#define _STM32LIB_DEVICE_MCP3204_H_

#include <stdint.h>
#include "../../hal/gpio.h"

typedef enum {
  MCP3204_CH0_SINGLE = 0x0600,
  MCP3204_CH1_SINGLE = 0x0640,
  MCP3204_CH2_SINGLE = 0x0680,
  MCP3204_CH3_SINGLE = 0x06c0
} MCP3204_ch;

typedef struct {
  SPI_TypeDef* spi;
  GPIO_Port csPort;
  GPIO_Pin csPin;
} MCP3204;

void mcp3204_setup(MCP3204* mcp3204);
uint16_t mcp3204_read(MCP3204* mcp3204, MCP3204_ch ch);

#endif
