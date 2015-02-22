
#include "usart.h"
#include "rcc.h"
#include "nvic.h"
#include "base.h"

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
  uint32_t apbclock;
  uint32_t divider;
  RCC_Clocks clocks;

  assert_param(IS_USART(initParams->instance));
  assert_param(IS_USART_BAUDRATE(initParams->baudRate));
  assert_param(IS_USART_WORD_LENGTH(initParams->wordLength));
  assert_param(IS_USART_STOP_BITS(initParams->stopBits));
  assert_param(IS_USART_PARITY(initParams->parity));
  assert_param(IS_USART_MODE(initParams->mode));
  assert_param(IS_USART_HARDWARE_FLOW_CONTROL(initParams->hardwareFlowControl));

  // disable
  initParams->instance->CR1 &= ~USART_CR1_UE;

  // CR1
  tmpReg = initParams->instance->CR1;

  tmpReg &= ~USART_PARITY_mask;
  tmpReg |= initParams->parity;

  tmpReg &= ~USART_WORD_LENGTH_mask;
  tmpReg |= initParams->wordLength;

  tmpReg &= ~USART_MODE_mask;
  tmpReg |= initParams->mode;

  initParams->instance->CR1 = tmpReg;

  // CR2
  tmpReg = initParams->instance->CR2;

  tmpReg &= ~USART_STOP_BITS_mask;
  tmpReg |= initParams->stopBits;

  initParams->instance->CR2 = tmpReg;

  // CR3
  tmpReg = initParams->instance->CR3;

  tmpReg &= ~USART_HARDWARE_FLOW_CONTROL_MASK;
  tmpReg |= initParams->hardwareFlowControl;

  initParams->instance->CR3 = tmpReg;

  /*---------------------------- USART BRR Configuration -----------------------*/
  /* Configure the USART Baud Rate -------------------------------------------*/
  RCC_getClocks(&clocks);

  if (initParams->instance == USART1) {
    apbclock = clocks.USART1CLK_Frequency;
  } else if (initParams->instance == USART2) {
    apbclock = clocks.USART2CLK_Frequency;
  } else if (initParams->instance == USART3) {
    apbclock = clocks.USART3CLK_Frequency;
  } else {
    apbclock = clocks.PCLK_Frequency;
  }

  /* Determine the integer part */
  if ((initParams->instance->CR1 & USART_CR1_OVER8) != 0) {
    /* (divider * 10) computing in case Oversampling mode is 8 Samples */
    divider = (uint32_t)((2 * apbclock) / (initParams->baudRate));
    tmpReg  = (uint32_t)((2 * apbclock) % (initParams->baudRate));
  } else { /* if ((initParams->instance->CR1 & CR1_OVER8_Set) == 0) */
    /* (divider * 10) computing in case Oversampling mode is 16 Samples */
    divider = (uint32_t)((apbclock) / (initParams->baudRate));
    tmpReg  = (uint32_t)((apbclock) % (initParams->baudRate));
  }

  /* round the divider : if fractional part i greater than 0.5 increment divider */
  if (tmpReg >= (initParams->baudRate) / 2) {
    divider++;
  }

  /* Implement the divider in case Oversampling mode is 8 Samples */
  if ((initParams->instance->CR1 & USART_CR1_OVER8) != 0) {
    /* get the LSB of divider and shift it to the right by 1 bit */
    tmpReg = (divider & (uint16_t)0x000F) >> 1;

    /* update the divider value */
    divider = (divider & (uint16_t)0xFFF0) | tmpReg;
  }

  /* Write to USART BRR */
  initParams->instance->BRR = (uint16_t)divider;
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

void USART_clearFlag(USART_Instance instance, USART_Flag flag) {
  assert_param(IS_USART(instance));
  assert_param(IS_USART_FLAG(flag));

  instance->ICR = (uint32_t)flag;
}

void USART_interruptsEnable(USART_Instance instance) {
  IRQn_Type irq;
  uint32_t imr;
  assert_param(IS_USART(instance));
  if (instance == USART1) {
    irq = USART1_IRQn;
    imr = EXTI_IMR_MR27;
  } else if (instance == USART2) {
    irq = USART2_IRQn;
    imr = EXTI_IMR_MR28;
  } else if (instance == USART3 || instance == USART4) {
    irq = USART3_4_IRQn;
    imr = EXTI_IMR_MR29;
  } else {
    assert_param(0);
    return;
  }

  NVIC_DisableIRQ(irq);
  
  EXTI->IMR = imr;

  NVIC_EnableIRQ(irq);
}

void USART_interruptTransmissionComplete(USART_Instance instance, FunctionalState state) {
  assert_param(IS_USART(instance));
  assert_param(IS_FUNCTIONAL_STATE(state));
  if (state == ENABLE) {
    instance->CR1 |= USART_CR1_TCIE;
  } else {
    instance->CR1 &= ~USART_CR1_TCIE;
  }
}

void USART_interruptReceive(USART_Instance instance, FunctionalState state) {
  assert_param(IS_USART(instance));
  assert_param(IS_FUNCTIONAL_STATE(state));
  if (state == ENABLE) {
    instance->CR1 |= USART_CR1_RXNEIE;
  } else {
    instance->CR1 &= ~USART_CR1_RXNEIE;
  }
}


