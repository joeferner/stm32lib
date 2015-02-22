
#include "platform_config.h"

extern void onUSARTTransmissionComplete();
extern void onUSARTReceive();

void SysTick_Handler() {
  time_SysTick_Handler();
}

void USART1_IRQHandler() {
  bool unexpectedIrq = true;
  USART_TypeDef *usart = DEBUG_USART;

  if (USART_getFlagStatus(DEBUG_USART, USART_Flag_TC)) {
    onUSARTTransmissionComplete();
    USART_clearFlag(DEBUG_USART, USART_Flag_TC);
    unexpectedIrq = false;
  }

  if (USART_getFlagStatus(DEBUG_USART, USART_Flag_RXNE)) {
    onUSARTReceive();
    USART_clearFlag(DEBUG_USART, USART_Flag_RXNE);
    unexpectedIrq = false;
  }

  if (unexpectedIrq) {
    while (1);
  }
}
