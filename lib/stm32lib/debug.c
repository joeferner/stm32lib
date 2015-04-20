
#include "debug.h"
#include "usart.h"
#include "iwdg.h"
#include "utils.h"
#include <platform_config.h>

#ifndef DEBUG_READ_INPUT_BUFFER_SIZE
#  define DEBUG_READ_INPUT_BUFFER_SIZE 100
#endif

#ifdef DEBUG_NETWORK_ENABLE
#include <net/ip/uip-udp-packet.h>
#include <net/ip/uiplib.h>
uip_ipaddr_t _debug_ipaddr;
static struct uip_udp_conn *_debug_udpconn = NULL;
#endif

#ifdef DEBUG_ENABLE_READ
#include "ringbuffer.h"
RingBufferU8 _debug_inputRingBuffer;
char _debug_inputBuffer[DEBUG_READ_INPUT_BUFFER_SIZE];
#endif

void debug_setup() {
  USART_InitParams usart;

#ifdef DEBUG_ENABLE_READ
  RingBufferU8_init(&_debug_inputRingBuffer, (uint8_t *)_debug_inputBuffer, DEBUG_READ_INPUT_BUFFER_SIZE);
#endif

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

#ifdef DEBUG_ENABLE_READ_IRQ
  USART_interruptReceive(DEBUG_USART, ENABLE);
  USART_interruptsEnable(DEBUG_USART);
#endif

  USART_enable(DEBUG_USART);

  IWDG_RESET;
}

#ifdef DEBUG_NETWORK_ENABLE
void debug_networkSetup() {
  uiplib_ipaddrconv(DEBUG_NETWORK_IP, &_debug_ipaddr);
  _debug_udpconn = udp_new(&_debug_ipaddr, UIP_HTONS(DEBUG_NETWORK_PORT), NULL);
}
#endif

uint8_t debug_rx() {
  while (!USART_rxHasData(DEBUG_USART));
  return USART_rx(DEBUG_USART);
}

void debug_tx(char* ptr, int len) {
  int n;
  char *p;
  for (n = 0, p = ptr; n < len; n++) {
    USART_txWaitForComplete(DEBUG_USART);
    USART_tx(DEBUG_USART, *p++);
  }
  
#ifdef DEBUG_NETWORK_ENABLE
  if(_debug_udpconn){
    uip_udp_packet_sendto(_debug_udpconn, ptr, len, &_debug_ipaddr, UIP_HTONS(DEBUG_NETWORK_PORT));
  }
#endif
}

#ifdef DEBUG_ENABLE_READ

void debug_tick() {
  char buffer[DEBUG_READ_INPUT_BUFFER_SIZE];

#ifndef DEBUG_ENABLE_READ_IRQ
  uint8_t b;
  while (USART_rxHasData(DEBUG_USART)) {
    b = USART_rx(DEBUG_USART);
    RingBufferU8_writeByte(&_debug_inputRingBuffer, b);
  }
#endif

  while (RingBufferU8_readLine(&_debug_inputRingBuffer, buffer, DEBUG_READ_INPUT_BUFFER_SIZE) > 0) {
    strTrim(buffer);
    debug_handleCommand(buffer);
    IWDG_RESET;
  }
  IWDG_RESET;
}

#ifdef DEBUG_ENABLE_READ_IRQ
void debug_usartIrq() {
  uint8_t b;
  if (USART_getFlagStatus(DEBUG_USART, USART_Flag_RXNE)) {
    USART_clearFlag(DEBUG_USART, USART_Flag_RXNE);
    b = USART_rx(DEBUG_USART);
    RingBufferU8_writeByte(&_debug_inputRingBuffer, b);
  }
}
#endif

#endif

