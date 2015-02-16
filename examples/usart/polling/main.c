
#include "platform_config.h"

static void setup();
static void loop();

int main(void) {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

static void setup() {
  USART_InitParams usart;

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

  USART_enable(DEBUG_USART);

  USART_txString(DEBUG_USART, "setup complete!\n");
}

static void loop() {
  uint8_t b;

  if (USART_rxHasData(DEBUG_USART)) {
    b = USART_rx(DEBUG_USART);
    USART_txWaitForComplete(DEBUG_USART);
    USART_tx(DEBUG_USART, b);
  }
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1);
}
