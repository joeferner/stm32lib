
#ifndef _STM32LIB_DEBUG_H_
#define _STM32LIB_DEBUG_H_

#include <stdint.h>
#include "platform_config.h"

#if defined(DEBUG_NETWORK_IP) && defined(DEBUG_NETWORK_PORT)
#  define DEBUG_NETWORK_ENABLE
#endif

void debug_setup();
#ifdef DEBUG_NETWORK_ENABLE
void debug_networkSetup();
#endif
uint8_t debug_rx();
void debug_tx(char* ptr, int len);

#ifdef DEBUG_ENABLE_READ
void debug_tick();
extern void debug_handleCommand(const char *str);

#ifdef DEBUG_ENABLE_READ_IRQ
void debug_usartIrq();
#endif

#endif

#endif

