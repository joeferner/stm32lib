
#include "platform_config.h"
#include <stdio.h>

volatile uint32_t bytesTransmitted;
volatile uint32_t bytesReceived;
volatile uint32_t lastPrint;

static void setup();
static void loop();
void onUSARTTransmissionComplete();
void onUSARTReceive();

int main() {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

static void setup() {
  USART_InitParams usart;

  lastPrint = 0;
  bytesTransmitted = 0;
  bytesReceived = 0;
  time_setup();

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

  USART_interruptTransmissionComplete(DEBUG_USART, ENABLE);
  USART_interruptReceive(DEBUG_USART, ENABLE);
  USART_interruptsEnable(DEBUG_USART);

  USART_txString(DEBUG_USART, "setup complete!\n");
}

static void loop() {
  if ((time_ms() - lastPrint) > 5000) {
    printf("tx: %lu, rx: %lu\n", bytesTransmitted, bytesReceived);
    lastPrint = time_ms();
  }
}

void onUSARTTransmissionComplete() {
  bytesTransmitted++;
}

void onUSARTReceive() {
  uint8_t b;

  while (USART_getFlagStatus(DEBUG_USART, USART_Flag_RXNE)) {
    bytesReceived++;
    b = USART_rx(DEBUG_USART);
    USART_txWaitForComplete(DEBUG_USART);
    USART_tx(DEBUG_USART, b);
  }
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1);
}

