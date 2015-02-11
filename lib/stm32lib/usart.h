
#ifndef _STM32LIB_USART_H_
#define _STM32LIB_USART_H_

#include "hal/usart.h"
#include "hal/gpio.h"

typedef struct {
  HAL_USART_InitParams halUsartInitParams;
  GPIO_Port txPort;
  GPIO_Pin txPin;
  GPIO_Port rxPort;
  GPIO_Pin rxPin;
} USART_InitParams;

void USART_initParamsInit(USART_InitParams *initParams);
void USART_init(USART_InitParams *initParams);
void USART_txString(USART_Instance instance, const char *str);
void USART_txBytes(USART_Instance instance, uint8_t *data, uint32_t offset, uint32_t len);

#endif
