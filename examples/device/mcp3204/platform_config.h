
#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

#define MCP3204_SPI       SPI1
#define MCP3204_MOSI_PORT GPIOA
#define MCP3204_MOSI_PIN  GPIO_Pin_7
#define MCP3204_MISO_PORT GPIOA
#define MCP3204_MISO_PIN  GPIO_Pin_6
#define MCP3204_SCK_PORT  GPIOA
#define MCP3204_SCK_PIN   GPIO_Pin_5
#define MCP3204_CS_PORT   GPIOB
#define MCP3204_CS_PIN    GPIO_Pin_6

#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/gpio.h>
#include <stm32lib/debug.h>
#include <stm32lib/rcc.h>
#include <stm32lib/time.h>
#include <stm32lib/spi.h>
#include <stm32lib/device/mcp3204/mcp3204.h>
