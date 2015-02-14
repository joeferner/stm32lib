
#include "debug.h"

#ifdef DEBUG_ENABLED

void debug_setup() {
  USART_InitParams usart;

  USART_initParamsInit(&usart);
  usart.txPort = DEBUG_TX_PORT;
  usart.txPin = DEBUG_TX_PIN;
  usart.rxPort = DEBUG_RX_PORT;
  usart.rxPin = DEBUG_RX_PIN;
  usart.halUsartInitParams.instance = DEBUG_USART;
  usart.halUsartInitParams.baudRate = DEBUG_BAUD;
  usart.halUsartInitParams.wordLength = USART_WordLength_8b;
  usart.halUsartInitParams.parity = USART_Parity_no;
  usart.halUsartInitParams.stopBits = USART_StopBits_1;
  usart.halUsartInitParams.hardwareFlowControl = USART_HardwareFlowControl_none;
  usart.halUsartInitParams.mode = USART_Mode_rx | USART_Mode_tx;
  USART_init(&usart);

  USART_enable(DEBUG_USART);
}

uint8_t debug_rx() {
  while(!USART_rxHasData(DEBUG_USART));
  return USART_rx(DEBUG_USART);
}

void debug_tx(uint8_t b) {
  USART_txWaitForComplete(DEBUG_USART);
  USART_tx(DEBUG_USART, b);
}

#endif
