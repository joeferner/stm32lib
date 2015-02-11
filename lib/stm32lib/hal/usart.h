
#ifndef _STM32LIB_HAL_USART_H_
#define _STM32LIB_HAL_USART_H_

#include "base.h"

typedef USART_TypeDef* USART_Instance;

typedef enum {
  USART_WordLength_8b = 1 // TODO
} USART_WordLength;

typedef enum {
  USART_Parity_no = 0 // TODO
} USART_Parity;

typedef enum {
  USART_StopBits_1 = 1 // TODO
} USART_StopBits;

typedef enum {
  USART_HardwareFlowControl_none = 0 // TODO
} USART_HardwareFlowControl;

typedef enum {
  USART_Mode_tx = 0, // TODO
  USART_Mode_rx = 1 // TODO
} USART_Mode;

typedef struct {
  USART_TypeDef* instance;
  uint32_t baudRate;
  USART_WordLength wordLength;
  USART_Parity parity;
  USART_StopBits stopBits;
  USART_HardwareFlowControl hardwareFlowControl;
  USART_Mode mode;
} USART_InitParams;

void USART_initParamsInit(USART_InitParams* initParams);
void USART_init(USART_InitParams* initParams);
void USART_enable(USART_Instance instance);
bool USART_rxHasData(USART_Instance instance);
uint8_t USART_rx(USART_Instance instance);
void USART_tx(USART_Instance instance, uint8_t b);
void USART_txWaitForComplete(USART_Instance instance);

#endif
