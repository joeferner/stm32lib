
#include "gpio.h"

void GPIO_initParamsInit(GPIO_InitParams *initParams) {
  initParams->port = NULL;
  initParams->pin  = GPIO_Pin_All;
  initParams->speed = GPIO_Speed_low;
  initParams->mode = GPIO_Mode_default;
  initParams->outputType = GPIO_OutputType_default;
  initParams->pullUpDown = GPIO_PullUpDown_no;
}

void GPIO_init(GPIO_InitParams *initParams) {
  uint32_t pinPos = 0;
  uint32_t pos = 0;
  uint32_t pos2bit = 0;
  uint32_t tmpMODER = 0;
  uint32_t tmpOTYPER = 0;
  uint32_t tmpOSPEEDR = 0;
  uint32_t tmpPUPDR = 0;
  uint32_t pinMask = 0;
  uint32_t pin2bitMask = 0;
  assert_param(IS_GPIO_PORT(initParams->port));
  assert_param(IS_GPIO_MODE(initParams->mode));
  assert_param(IS_GPIO_OUTPUT_TYPE(initParams->outputType));
  assert_param(IS_GPIO_SPEED(initParams->speed));
  assert_param(IS_GPIO_PULL_UP_DOWN(initParams->pullUpDown));

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
  uint32_t pinPos;
  uint32_t pos;
  uint32_t tmp;

  assert_param(af <= 0xf);
  
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
}


