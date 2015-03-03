
#ifndef PLATFORM_CONFIG_H_INCLUDED
#define PLATFORM_CONFIG_H_INCLUDED

#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

#define SDCARD_SPI       SPI2
#define SDCARD_CS_PORT   GPIOB
#define SDCARD_CS_PIN    GPIO_Pin_1
#define SDCARD_SCK_PORT  GPIOB
#define SDCARD_SCK_PIN   GPIO_Pin_13
#define SDCARD_MISO_PORT GPIOB
#define SDCARD_MISO_PIN  GPIO_Pin_14
#define SDCARD_MOSI_PORT GPIOB
#define SDCARD_MOSI_PIN  GPIO_Pin_15

#include <stm32lib/utils.h>
#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/gpio.h>
#include <stm32lib/debug.h>
#include <stm32lib/rcc.h>
#include <stm32lib/time.h>
#include <stm32lib/device/sdcard/sdcard.h>
#include <stm32lib/device/sdcard/fat.h>

#endif /* PLATFORM_CONFIG_H_INCLUDED */
