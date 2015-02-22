
#ifndef _STM32LIB_DEBUG_H_
#define _STM32LIB_DEBUG_H_

#include <stdint.h>

void debug_setup();
uint8_t debug_rx();
void debug_tx(uint8_t b);

#endif

