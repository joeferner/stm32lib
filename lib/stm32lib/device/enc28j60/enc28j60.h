
#ifndef _STM32LIB_DEVICE_ENC28J60_H_
#define _STM32LIB_DEVICE_ENC28J60_H_

#include <stdint.h>
#include <sys/timer.h>
#include "../../hal/gpio.h"
#include "../../spi.h"

typedef struct {
  SPI_TypeDef *spi;
  GPIO_Port csPort;
  GPIO_Pin csPin;
  GPIO_Port resetPort;
  GPIO_Pin resetPin;
  uint8_t macAddress[6];
  struct timer _enc28j60_periodicTimer;
  uint16_t _enc28j60_nextPacketPtr;
  uint8_t _enc28j60_bank;
} ENC28J60;

void enc28j60_setup(ENC28J60 *enc28j60);
void enc28j60_tick(ENC28J60 *enc28j60);

void enc28j60_send(ENC28J60 *enc28j60);
extern uint8_t enc28j60_tcp_output();

#endif
