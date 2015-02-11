
#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/usart.h>
#include <stm32lib/hal/gpio.h>

#define DEBUG_RCC1    0
#define DEBUG_RCC2    RCC_peripheral2_GPIOA | RCC_peripheral2_AFIO | RCC_peripheral2_USART1
#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

