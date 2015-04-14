
#include "usart.h"
#include "rcc.h"
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

#ifdef STM32F0XX
  uint32_t divider;

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

#elif defined(STM32F10X)
  uint32_t integerdivider;
  uint32_t fractionaldivider;

  if (initParams->instance == USART1) {
    apbclock = clocks.PCLK2_Frequency;
  } else {
    apbclock = clocks.PCLK1_Frequency;
  }

  /* Determine the integer part */
  if ((initParams->instance->CR1 & USART_CR1_OVER8) != 0) {
    /* Integer part computing in case Oversampling mode is 8 Samples */
    integerdivider = ((25 * apbclock) / (2 * (initParams->baudRate)));
  } else {
    /* Integer part computing in case Oversampling mode is 16 Samples */
    integerdivider = ((25 * apbclock) / (4 * (initParams->baudRate)));
  }
  tmpReg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpReg >> 4));

  /* Implement the fractional part in the register */
  if ((initParams->instance->CR1 & USART_CR1_OVER8) != 0) {
    tmpReg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  } else {
    tmpReg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }

  /* Write to USART BRR */
  initParams->instance->BRR = (uint16_t)tmpReg;
#else
#  error "No valid chip defined"
#endif
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
    instance->CR1 |= USART_CR1_UE;
  } else {
    instance->CR1 &= ~USART_CR1_UE;
  }
}

bool USART_rxHasData(USART_Instance instance) {
  return USART_getFlagStatus(instance, USART_Flag_RXNE) != RESET;
}

uint16_t USART_rx(USART_Instance instance) {
  assert_param(IS_USART(instance));
#ifdef STM32F0XX
  return (uint16_t)(instance->RDR & (uint16_t)0x01FF);
#elif defined (STM32F10X)
  return (uint16_t)(instance->DR & (uint16_t)0x01FF);
#else
#  error "No valid chip defined"
#endif
}

void USART_tx(USART_Instance instance, uint8_t b) {
  assert_param(IS_USART(instance));
  assert_param(IS_USART_DATA(b));
#ifdef STM32F0XX
  instance->TDR = (b & (uint16_t)0x01FF);
#elif defined (STM32F10X)
  instance->DR = (b & (uint16_t)0x01FF);
#else
#  error "No valid chip defined"
#endif
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

#ifdef STM32F0XX
  uint32_t sr = instance->ISR;
#elif defined (STM32F10X)
  uint32_t sr = instance->SR;
#else
#  error "No valid chip defined"
#endif

  if ((sr & flag) != RESET) {
    return SET;
  } else {
    return RESET;
  }
}

void USART_clearFlag(USART_Instance instance, USART_Flag flag) {
  assert_param(IS_USART(instance));
  assert_param(IS_USART_FLAG(flag));

#ifdef STM32F0XX
  instance->ICR = (uint32_t)flag;
#elif defined (STM32F10X)
  instance->SR &= ~((uint32_t)flag);
#else
#  error "No valid chip defined"
#endif
}

void USART_interruptsEnable(USART_Instance instance) {
  IRQn_Type irq;
  uint32_t imr;
  assert_param(IS_USART(instance));

  irq = USART_getIrqForPort(instance);

#ifdef STM32F0XX
  if (instance == USART1) {
    imr = EXTI_IMR_MR27;
  } else if (instance == USART2) {
    imr = EXTI_IMR_MR28;
  } else if (instance == USART3 || instance == USART4) {
    imr = EXTI_IMR_MR29;
  } else {
    assert_param(0);
    return;
  }
#elif defined (STM32F10X)
  imr = 0;
#else
#  error "No valid chip defined"
#endif

  NVIC_DisableIRQ(irq);

  if (imr > 0) {
    EXTI->IMR = imr;
  }

  NVIC_EnableIRQ(irq);
}

IRQn_Type USART_getIrqForPort(USART_Instance instance) {
#ifdef STM32F0XX
  if (instance == USART1) {
    return USART1_IRQn;
  } else if (instance == USART2) {
    return USART2_IRQn;
  } else if (instance == USART3 || instance == USART4) {
    return USART3_4_IRQn;
  } else {
    assert_param(0);
    return;
  }
#elif defined (STM32F10X)
  if (instance == USART1) {
    return USART1_IRQn;
  } else if (instance == USART2) {
    return USART2_IRQn;
  } else if (instance == USART3) {
    return USART3_IRQn;
  } else {
    assert_param(0);
    return -1;
  }
#else
#  error "No valid chip defined"
#endif
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


