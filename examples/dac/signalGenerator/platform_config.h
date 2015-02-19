
#define DEBUG_ENABLED
#define DEBUG_RCC     RCC_peripheral_GPIOA | RCC_peripheral_AFIO | RCC_peripheral_USART1
#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

#define DAC_PORT      GPIOA
#define DAC_PIN       GPIO_Pin_4

#include <stm32lib/utils.h>
#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/gpio.h>
#include <stm32lib/debug.h>
#include <stm32lib/rcc.h>
#include <stm32lib/time.h>
#include <stm32lib/dac.h>
