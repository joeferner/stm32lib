
#ifndef _STM32LIB_DEBUG_H_
#define _STM32LIB_DEBUG_H_

#include <stdint.h>
#include "platform_config.h"

void debug_setup();
uint8_t debug_rx();
void debug_tx(uint8_t b);

#ifdef DEBUG_ENABLE_READ
void debug_tick();
extern void debug_handleCommand(const char* str);
#endif

#endif

