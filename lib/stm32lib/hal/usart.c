
#include "usart.h"
#include "rcc.h"

/** @brief  BRR division operation to set BRR register in 8-bit oversampling mode
  * @param  _PCLK_: UART clock
  * @param  _BAUD_: Baud rate set by the user
  * @retval Division result
  */
#define __DIV_SAMPLING8(_PCLK_, _BAUD_)             (((_PCLK_)*2)/((_BAUD_)))

/** @brief  BRR division operation to set BRR register in 16-bit oversampling mode
  * @param  _PCLK_: UART clock
  * @param  _BAUD_: Baud rate set by the user
  * @retval Division result
  */
#define __DIV_SAMPLING16(_PCLK_, _BAUD_)             (((_PCLK_))/((_BAUD_)))

typedef enum {
  USART_ClockSource_pclk = 0b00,
  USART_ClockSource_sysclk = 0b01,
  USART_ClockSource_lse = 0b10,
  USART_ClockSource_hsi = 0b11
} USART_ClockSource;

void HAL_USART_initParamsInit(HAL_USART_InitParams *initParams) {
  initParams->baudRate = 9600;
  initParams->wordLength = USART_WordLength_8b;
  initParams->stopBits = USART_StopBits_1;
  initParams->parity = USART_Parity_no;
  initParams->mode = USART_Mode_rx | USART_Mode_tx;
  initParams->hardwareFlowControl = USART_HardwareFlowControl_none;
}

void HAL_USART_init(HAL_USART_InitParams *initParams) {
  uint32_t tmpReg = 0x00;
  uint32_t clockDiv = 0x00;
  uint32_t brrTemp = 0;
  USART_ClockSource clocksource = USART_ClockSource_lse;

  assert_param(IS_USART(initParams->instance));
  assert_param(IS_USART_BAUDRATE(initParams->baudRate));
  assert_param(IS_USART_WORD_LENGTH(initParams->wordLength));
  assert_param(IS_USART_STOP_BITS(initParams->stopBits));
  assert_param(IS_USART_PARITY(initParams->parity));
  assert_param(IS_USART_MODE(initParams->mode));
  assert_param(IS_USART_HARDWARE_FLOW_CONTROL(initParams->hardwareFlowControl));

  // CR1
  tmpReg = initParams->instance->CR1;

  tmpReg &= ~USART_STOP_BITS_mask;
  tmpReg |= initParams->stopBits;

  tmpReg &= ~USART_PARITY_mask;
  tmpReg |= initParams->parity;

  tmpReg &= ~USART_WORD_LENGTH_mask;
  tmpReg |= initParams->wordLength;

  tmpReg &= ~USART_MODE_mask;
  tmpReg |= initParams->mode;

  initParams->instance->CR1 = tmpReg;

  // CR3
  tmpReg = initParams->instance->CR3;

  tmpReg &= ~USART_HARDWARE_FLOW_CONTROL_MASK;
  tmpReg |= initParams->hardwareFlowControl;

  initParams->instance->CR3 = tmpReg;

  /*---------------------------- USART BRR Configuration -----------------------*/
  if (initParams->instance == USART1) {
    clocksource = RCC->CFGR3 & 0b11;
  } else {
    assert_param(0);
  }

  if (initParams->instance->CR1 & USART_CR1_OVER8) {
    switch (clocksource) {
    case USART_ClockSource_pclk:
      clockDiv = (uint16_t)(__DIV_SAMPLING8(RCC_getPCLK1Freq(), initParams->baudRate));
      break;
    case USART_ClockSource_hsi:
      clockDiv = (uint16_t)(__DIV_SAMPLING8(HSI_VALUE, initParams->baudRate));
      break;
    case USART_ClockSource_sysclk:
      clockDiv = (uint16_t)(__DIV_SAMPLING8(RCC_getSysClockFreq(), initParams->baudRate));
      break;
    case USART_ClockSource_lse:
      clockDiv = (uint16_t)(__DIV_SAMPLING8(LSE_VALUE, initParams->baudRate));
      break;
    }

    brrTemp = clockDiv & 0xFFF0;
    brrTemp |= (uint16_t)((clockDiv & (uint16_t)0x000F) >> 1U);
    initParams->instance->BRR = brrTemp;
  } else {
    switch (clocksource) {
    case USART_ClockSource_pclk:
      initParams->instance->BRR = (uint16_t)(__DIV_SAMPLING16(RCC_getPCLK1Freq(), initParams->baudRate));
      break;
    case USART_ClockSource_hsi:
      initParams->instance->BRR = (uint16_t)(__DIV_SAMPLING16(HSI_VALUE, initParams->baudRate));
      break;
    case USART_ClockSource_sysclk:
      initParams->instance->BRR = (uint16_t)(__DIV_SAMPLING16(RCC_getSysClockFreq(), initParams->baudRate));
      break;
    case USART_ClockSource_lse:
      initParams->instance->BRR = (uint16_t)(__DIV_SAMPLING16(LSE_VALUE, initParams->baudRate));
      break;
    }
  }
}

void USART_enable(USART_Instance instance) {
  USART_setState(instance, ENABLE);
}

void USART_disable(USART_Instance instance) {
  USART_setState(instance, DISABLE);
}

void USART_setState(USART_Instance instance, FunctionalState state) {
  assert_param(IS_USART(instance));
  assert_param(IS_FUNCTIONAL_STATE(state));

  if (state != DISABLE) {
    instance->CR1 |= 0b1;
  } else {
    instance->CR1 &= 0b0;
  }
}

bool USART_rxHasData(USART_Instance instance) {
  return USART_getFlagStatus(instance, USART_Flag_RXNE) != RESET;
}

uint16_t USART_rx(USART_Instance instance) {
  assert_param(IS_USART(instance));
  return (uint16_t)(instance->RDR & (uint16_t)0x01FF);
}

void USART_tx(USART_Instance instance, uint8_t b) {
  assert_param(IS_USART(instance));
  assert_param(IS_USART_DATA(b));
  instance->TDR = (b & (uint16_t)0x01FF);
}

bool USART_txComplete(USART_Instance instance) {
  return USART_getFlagStatus(instance, USART_Flag_TXE) != RESET;
}

void USART_txWaitForComplete(USART_Instance instance) {
  while (!USART_txComplete(instance));
}

FlagStatus USART_getFlagStatus(USART_Instance instance, USART_Flag flag) {
  assert_param(IS_USART(instance));
  assert_param(IS_USART_FLAG(flag));

  if ((instance->ISR & flag) != RESET) {
    return SET;
  } else {
    return RESET;
  }
}
