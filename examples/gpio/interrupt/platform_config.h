
#define DEBUG_USART   USART1
#define DEBUG_BAUD    9600
#define DEBUG_TX_PORT GPIOA
#define DEBUG_TX_PIN  GPIO_Pin_9
#define DEBUG_RX_PORT GPIOA
#define DEBUG_RX_PIN  GPIO_Pin_10

#define BUTTON_PORT   GPIOC
#define BUTTON_PIN    GPIO_Pin_13

#include <stm32lib/utils.h>
#include <stm32lib/hal/rcc.h>
#include <stm32lib/hal/gpio.h>
#include <stm32lib/exti.h>
#include <stm32lib/debug.h>
#include <stm32lib/rcc.h>
#include <stm32lib/time.h>
