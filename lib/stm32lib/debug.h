
#ifndef _STM32LIB_DEBUG_H_
#define _STM32LIB_DEBUG_H_

#include <platform_config.h>

#ifdef DEBUG_ENABLED

#include <stm32lib/usart.h>

void debug_setup();
uint8_t debug_rx();
void debug_tx(uint8_t b);

#endif
#endif

