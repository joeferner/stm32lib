
#include "gpio.h"

uint32_t _GPIO_portToEXTIPortSource(GPIO_Port port);
uint32_t _GPIO_pinToEXTIPinSource(GPIO_Pin pin);

void GPIO_initParamsInit(GPIO_InitParams *initParams) {
  initParams->port = NULL;
  initParams->pin  = 0;
  initParams->speed = GPIO_Speed_low;
  initParams->mode = GPIO_Mode_default;
  initParams->outputType = GPIO_OutputType_default;
  initParams->pullUpDown = GPIO_PullUpDown_no;
}

void GPIO_init(GPIO_InitParams *initParams) {
  assert_param(IS_GPIO_PORT(initParams->port));
  assert_param(IS_GPIO_MODE(initParams->mode));
  assert_param(IS_GPIO_OUTPUT_TYPE(initParams->outputType));
  assert_param(IS_GPIO_SPEED(initParams->speed));
  assert_param(IS_GPIO_PULL_UP_DOWN(initParams->pullUpDown));

  uint32_t pinPos = 0;
  uint32_t pos = 0;
#ifdef STM32F0XX
  uint32_t pinMask = 0;
  uint32_t pos2bit = 0;
  uint32_t pin2bitMask = 0;
  uint32_t tmpMODER = 0;
  uint32_t tmpOTYPER = 0;
  uint32_t tmpOSPEEDR = 0;
  uint32_t tmpPUPDR = 0;

  tmpMODER = initParams->port->MODER;
  tmpOTYPER = initParams->port->OTYPER;
  tmpOSPEEDR = initParams->port->OSPEEDR;
  tmpPUPDR = initParams->port->PUPDR;
  for (pinPos = 0; pinPos < 16; pinPos++) {
    pos = 1 << pinPos;
    if (initParams->pin & pos) {
      pos2bit = pinPos * 2;
      pin2bitMask = 0b11 << pos2bit;
      pinMask = 0b1 << pos;

      tmpMODER &= ~pin2bitMask;
      tmpMODER |= initParams->mode << pos2bit;

      tmpOTYPER &= ~pinMask;
      tmpOTYPER |= initParams->outputType << pos;

      tmpOSPEEDR &= ~pin2bitMask;
      tmpOSPEEDR |= initParams->speed << pos2bit;

      tmpPUPDR &= ~pin2bitMask;
      tmpPUPDR |= initParams->pullUpDown << pos2bit;
    }
  }
  initParams->port->MODER = tmpMODER;
  initParams->port->OTYPER = tmpOTYPER;
  initParams->port->OSPEEDR = tmpOSPEEDR;
  initParams->port->PUPDR = tmpPUPDR;

#elif defined (STM32F10X)
  bool lowReg = false;
  uint32_t pos4bit = 0;
  uint32_t pin4bitMask = 0;
  uint32_t tmpCRL = 0;
  uint32_t tmpCRH = 0;
  uint8_t tmp = 0;

  tmpCRL = initParams->port->CRL;
  tmpCRH = initParams->port->CRH;
  for (pinPos = 0; pinPos < 16; pinPos++) {
    pos = 1 << pinPos;
    if (initParams->pin & pos) {
      lowReg = pinPos < 8;
      pos4bit = (pinPos - (lowReg ? 0 : 8)) * 4;
      pin4bitMask = 0b1111 << pos4bit;

      tmp = 0;
      if (initParams->mode == GPIO_Mode_input || initParams->mode == GPIO_Mode_alternateFunctionInput) {
        if (initParams->pullUpDown == GPIO_PullUpDown_no) {
          tmp |= 0b0100;
        } else {
          tmp |= 0b1000;
        }
      } else if (initParams->mode == GPIO_Mode_output || initParams->mode == GPIO_Mode_alternateFunctionOutput) {
        if (initParams->speed == GPIO_Speed_low) {
          tmp |= 0b0010;
        } else if (initParams->speed == GPIO_Speed_medium) {
          tmp |= 0b0001;
        } else {
          tmp |= 0b0011;
        }

        if (initParams->mode == GPIO_Mode_alternateFunctionOutput) {
          if (initParams->outputType == GPIO_OutputType_openDrain) {
            tmp |= 0b1100;
          } else {
            tmp |= 0b1000;
          }
        } else {
          if (initParams->outputType == GPIO_OutputType_openDrain) {
            tmp |= 0b0100;
          }
        }
      } else if (initParams->mode == GPIO_Mode_analog) {
        tmp |= 0b0000;
      }

      if (lowReg) {
        tmpCRL &= ~pin4bitMask;
        tmpCRL |= tmp << pos4bit;
      } else {
        tmpCRH &= ~pin4bitMask;
        tmpCRH |= tmp << pos4bit;
      }
    }
  }

  initParams->port->CRL = tmpCRL;
  initParams->port->CRH = tmpCRH;
#else
#  error "No valid chip defined"
#endif
}

void GPIO_setBits(GPIO_Port port, GPIO_Pin pin) {
  assert_param(IS_GPIO_PORT(port));
  port->BSRR = pin;
}

void GPIO_resetBits(GPIO_Port port, GPIO_Pin pin) {
  assert_param(IS_GPIO_PORT(port));
  port->BRR = pin;
}

void GPIO_writeBits(GPIO_Port port, GPIO_Pin pin, GPIO_BitAction bitAction) {
  assert_param(IS_GPIO_PORT(port));
  assert_param(IS_GPIO_BIT_ACTION(bitAction));
  if (bitAction != GPIO_Bit_reset) {
    port->BSRR = pin;
  } else {
    port->BRR = pin;
  }
}

GPIO_BitAction GPIO_readInputBit(GPIO_Port port, GPIO_Pin pin) {
  assert_param(IS_GPIO_PORT(port));
  if (port->IDR & pin) {
    return GPIO_Bit_set;
  } else {
    return GPIO_Bit_reset;
  }
}

void GPIO_setAlternateFunction(GPIO_Port port, GPIO_Pin pin, uint8_t af) {
  assert_param(af <= 0xf);

  uint32_t tmp;
#ifdef STM32F0XX
  uint32_t pinPos;
  uint32_t pos;

  tmp = port->AFR[0];
  for (pinPos = 0; pinPos < 8; pinPos++) {
    pos = 1 << pinPos;
    if (pin & pos) {
      tmp &= ~(0xF << (pinPos * 4));
      tmp |= (af << (pinPos * 4));
    }
  }
  port->AFR[0] = tmp;

  tmp = port->AFR[1];
  for (pinPos = 8; pinPos < 16; pinPos++) {
    pos = 1 << pinPos;
    if (pin & pos) {
      tmp &= ~(0xF << ((pinPos - 8) * 4));
      tmp |= (af << ((pinPos - 8) * 4));
    }
  }
  port->AFR[1] = tmp;
#elif defined (STM32F10X)
  if (af == 0) {
    if (port == GPIOA && pin == (GPIO_Pin_9 | GPIO_Pin_10)) { // USART1 - No Remap
      tmp = AFIO->MAPR;
      tmp &= ~AFIO_MAPR_USART1_REMAP;
      AFIO->MAPR = tmp;
      return;
    } else if (port == GPIOA && pin == (GPIO_Pin_2 | GPIO_Pin_3)) { // USART2 - No Remap
      tmp = AFIO->MAPR;
      tmp &= ~AFIO_MAPR_USART2_REMAP;
      AFIO->MAPR = tmp;
      return;
    } else if (port == GPIOA && pin == (GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7)) { // SPI1 - No Remap
      tmp = AFIO->MAPR;
      tmp &= ~AFIO_MAPR_SPI1_REMAP;
      AFIO->MAPR = tmp;
      return;
    } else if (port == GPIOB && pin == (GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)) { // SPI2 - No Remap
      return; // SPI2 doesn't remap
    } else if (port == GPIOB && pin == (GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5)) { // SPI3 - No Remap
#ifdef AFIO_MAPR_SPI3_REMAP
      tmp = AFIO->MAPR;
      tmp &= ~AFIO_MAPR_SPI3_REMAP;
      AFIO->MAPR = tmp;
#endif
      return;
    }
  }
  assert_param(0);
#else
#  error "No valid chip defined"
#endif
}

uint32_t _GPIO_portToEXTIPortSource(GPIO_Port port) {
  assert_param(IS_GPIO_PORT(port));

  if (port == GPIOA) {
    return 0x00;
  } else if (port == GPIOB) {
    return 0x01;
  } else if (port == GPIOC) {
    return 0x02;
  } else if (port == GPIOD) {
    return 0x03;
  } else if (port == GPIOE) {
    return 0x04;
  } else if (port == GPIOF) {
    return 0x05;
  }
  assert_param(0);
  return -1;
}

uint32_t _GPIO_pinToEXTIPinSource(GPIO_Pin pin) {
  assert_param(IS_GPIO_PIN(pin));

  switch (pin) {
  case GPIO_Pin_0: return 0x00;
  case GPIO_Pin_1: return 0x01;
  case GPIO_Pin_2: return 0x02;
  case GPIO_Pin_3: return 0x03;
  case GPIO_Pin_4: return 0x04;
  case GPIO_Pin_5: return 0x05;
  case GPIO_Pin_6: return 0x06;
  case GPIO_Pin_7: return 0x07;
  case GPIO_Pin_8: return 0x08;
  case GPIO_Pin_9: return 0x09;
  case GPIO_Pin_10: return 0x0a;
  case GPIO_Pin_11: return 0x0b;
  case GPIO_Pin_12: return 0x0c;
  case GPIO_Pin_13: return 0x0d;
  case GPIO_Pin_14: return 0x0e;
  case GPIO_Pin_15: return 0x0f;
  }
  assert_param(0);
  return -1;
}

void GPIO_EXTILineConfig(GPIO_Port port, GPIO_Pin pin) {
  uint32_t tmp;
  assert_param(IS_GPIO_PORT(port));
  assert_param(IS_GPIO_PIN(pin));

  uint32_t portSource = _GPIO_portToEXTIPortSource(port);
  uint32_t pinSource = _GPIO_pinToEXTIPinSource(pin);

#ifdef STM32F0XX
  tmp = ((uint32_t)0x0F) << (0x04 * (pinSource & (uint8_t)0x03));
  SYSCFG->EXTICR[pinSource >> 0x02] &= ~tmp;
  SYSCFG->EXTICR[pinSource >> 0x02] |= (((uint32_t)portSource) << (0x04 * (pinSource & (uint8_t)0x03)));
#elif defined (STM32F10X)
  assert_param(0);
#else
#  error "No valid chip defined"
#endif
}

void SWJTAG_setup(SWJTAG_State state) {
  assert_param(IS_SWJTAG_STATE(state));

#ifdef STM32F0XX
  assert_param(0);
#elif defined (STM32F10X)
  uint32_t tmp;

  tmp = AFIO->MAPR;
  tmp &= ~SWJTAG_State_mask;
  tmp |= state;
  AFIO->MAPR = tmp;
#else
#  error "No valid chip defined"
#endif
}
