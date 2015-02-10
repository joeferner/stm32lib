
#ifndef _STM32LIB_HAL_USART_H_
#define _STM32LIB_HAL_USART_H_

#include "base.h"

typedef enum {
  USART1 = 1 // TODO
} USART_instance;

typedef enum {
  USART_wordLength_8b = 1 // TODO
} USART_wordLength;

typedef enum {
  USART_parity_no = 0 // TODO
} USART_parity;

typedef enum {
  USART_stopBits_1 = 1 // TODO
} USART_stopBits;

typedef enum {
  USART_hardwareFlowControl_none = 0 // TODO
} USART_hardwareFlowControl;

typedef enum {
  USART_mode_tx = 0, // TODO
  USART_mode_rx = 1 // TODO
} USART_mode;

typedef struct {
  USART_instance instance;
  uint32_t baudRate;
  USART_wordLength wordLength;
  USART_parity parity;
  USART_stopBits stopBits;
  USART_hardwareFlowControl hardwareFlowControl;
  USART_mode mode;
} USART_initParams;

void USART_initParamsInit(USART_initParams* initParams);
void USART_init(USART_initParams* initParams);
void USART_enable(USART_instance instance);
bool USART_rxHasData(USART_instance instance);
uint8_t USART_rx(USART_instance instance);
void USART_tx(USART_instance instance, uint8_t b);
void USART_txWaitForComplete(USART_instance instance);

#endif
