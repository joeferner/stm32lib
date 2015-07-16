
#ifndef _NRF8001_USART_H_
#define _NRF8001_USART_H_

#include <stdbool.h>
#include "lib_aci.h"
#include "../../ringbuffer.h"

extern RingBufferU8 nrf8001_usart_rxBuffer;
extern struct aci_state_t aci_state;

void nrf8001_usart_setup(bool debug);
void nrf8001_usart_tick();
bool nrf8001_usart_tx(const uint8_t* buffer, uint16_t len);
extern void nrf8001_usart_rx();

#endif
