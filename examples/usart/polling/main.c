
#include "platform_config.h"

void setup();
void loop();

int main(void) {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

void setup() {
  GPIO_InitParams gpio;
  USART_InitParams usart;

  RCC_peripheral1ClockEnable(DEBUG_RCC1);
  RCC_peripheral2ClockEnable(DEBUG_RCC2);

  GPIO_initParamsInit(&gpio);
  gpio.port = DEBUG_TX_PORT;
  gpio.pin = DEBUG_TX_PIN;
  gpio.mode = GPIO_Mode_outputPushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  gpio.port = DEBUG_RX_PORT;
  gpio.pin = DEBUG_RX_PIN;
  gpio.mode = GPIO_Mode_input;
  GPIO_init(&gpio);
  
  USART_initParamsInit(&usart);
  usart.instance = DEBUG_USART;
  usart.baudRate = DEBUG_BAUD;
  usart.wordLength = USART_WordLength_8b;
  usart.parity = USART_Parity_no;
  usart.stopBits = USART_StopBits_1;
  usart.hardwareFlowControl = USART_HardwareFlowControl_none;
  usart.mode = USART_Mode_rx | USART_Mode_tx;
  USART_init(&usart);
  
  USART_enable(DEBUG_USART);
}

void loop() {
  uint8_t b;
  
  if(USART_rxHasData(DEBUG_USART)) {
    b = USART_rx(DEBUG_USART);
    USART_tx(DEBUG_USART, b);
    USART_txWaitForComplete(DEBUG_USART);
  }
}

