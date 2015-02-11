
#ifndef _STM32LIB_USART_H_
#define _STM32LIB_USART_H_

#include "hal/usart.h"

void USART_txString(USART_Instance instance, const char* str);
void USART_txBytes(USART_Instance instance, uint8_t* data, uint32_t offset, uint32_t len);

#endif
