
#ifndef PLATFORM_CONFIG_H_INCLUDED
#define PLATFORM_CONFIG_H_INCLUDED

#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

#define CC3000_SPI       SPI1
#define CC3000_MOSI_PORT GPIOA
#define CC3000_MOSI_PIN  GPIO_Pin_7
#define CC3000_MISO_PORT GPIOA
#define CC3000_MISO_PIN  GPIO_Pin_6
#define CC3000_SCK_PORT  GPIOA
#define CC3000_SCK_PIN   GPIO_Pin_5
#define CC3000_CS_PORT   GPIOB
#define CC3000_CS_PIN    GPIO_Pin_3
#define CC3000_IRQ_PORT  GPIOB
#define CC3000_IRQ_PIN   GPIO_Pin_4
#define CC3000_EN_PORT   GPIOB
#define CC3000_EN_PIN    GPIO_Pin_5

#include <stm32lib/utils.h>
#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/gpio.h>
#include <stm32lib/debug.h>
#include <stm32lib/rcc.h>
#include <stm32lib/time.h>
#include <stm32lib/device/cc3000/cc3000.h>

#endif /* PLATFORM_CONFIG_H_INCLUDED */
