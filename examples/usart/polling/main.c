
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
  GPIO_InitParams gpio;
  USART_InitParams usart;

  RCC_peripheralClockEnable(DEBUG_RCC);

  GPIO_initParamsInit(&gpio);
  gpio.port = DEBUG_TX_PORT;
  gpio.pin = DEBUG_TX_PIN;
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
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

  USART_txString(DEBUG_USART, "setup complete!\n");
}

static void loop() {
  uint8_t b;

  RCC_TypeDef *rcc = RCC;
  USART_TypeDef *usart1 = USART1;

  if (USART_rxHasData(DEBUG_USART)) {
    b = USART_rx(DEBUG_USART);
    USART_txWaitForComplete(DEBUG_USART);
    USART_tx(DEBUG_USART, b);
  }
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1);
}
