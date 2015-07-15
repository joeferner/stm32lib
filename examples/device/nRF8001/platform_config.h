
#ifndef PLATFORM_CONFIG_H_INCLUDED
#define PLATFORM_CONFIG_H_INCLUDED

#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

#define BLUETOOTH_SPI           SPI1
#define BLUETOOTH_SPI_RCC       RCC_Peripheral_SPI1
#define BLUETOOTH_SPI_PRESCALER SPI_BaudRatePrescaler_256
#define BLUETOOTH_RDYN_PORT     GPIOC
#define BLUETOOTH_RDYN_PIN      GPIO_Pin_11
#define BLUETOOTH_RDYN_EXTI     EXTI_Line_11
#define BLUETOOTH_MOSI_PORT     GPIOB
#define BLUETOOTH_MOSI_PIN      GPIO_Pin_5
#define BLUETOOTH_MISO_PORT     GPIOB
#define BLUETOOTH_MISO_PIN      GPIO_Pin_4
#define BLUETOOTH_SCK_PORT      GPIOB
#define BLUETOOTH_SCK_PIN       GPIO_Pin_3
#define BLUETOOTH_CS_PORT       GPIOA
#define BLUETOOTH_CS_PIN        GPIO_Pin_15
#define BLUETOOTH_RESET_PORT    GPIOC
#define BLUETOOTH_RESET_PIN     GPIO_Pin_10
#define BLUETOOTH_ACTIVE_PORT   GPIOC
#define BLUETOOTH_ACTIVE_PIN    GPIO_Pin_12

#include <stm32lib/utils.h>
#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/gpio.h>
#include <stm32lib/debug.h>
#include <stm32lib/rcc.h>
#include <stm32lib/time.h>

#endif /* PLATFORM_CONFIG_H_INCLUDED */
