
#include "usart.h"
#include "rcc.h"

void USART_initParamsInit(USART_InitParams *initParams) {
  HAL_USART_initParamsInit(&initParams->halUsartInitParams);
  initParams->txPort = NULL;
  initParams->txPin = -1;
  initParams->rxPort = NULL;
  initParams->rxPin = -1;
}

void USART_init(USART_InitParams *initParams) {
  GPIO_InitParams gpio;

  // RCC
  RCC_peripheralClockEnable(RCC_Peripheral_AFIO);
  if (initParams->halUsartInitParams.instance == USART1) {
    RCC_peripheralClockEnable(RCC_Peripheral_USART1);
  } else {
    assert_param(0);
  }
  RCC_peripheralClockEnableForPort(initParams->txPort);
  RCC_peripheralClockEnableForPort(initParams->rxPort);

  // GPIO AFIO
#ifdef STM32F0XX
  if (initParams->halUsartInitParams.instance == USART1
      && initParams->txPort == GPIOA && initParams->txPin == GPIO_Pin_9
      && initParams->rxPort == GPIOA && initParams->rxPin == GPIO_Pin_10) {
    GPIO_setAlternateFunction(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, 1);
  } else {
    assert_param(0);
  }
#elif defined(STM32F10X)
  if (initParams->halUsartInitParams.instance == USART1
      && initParams->txPort == GPIOA && initParams->txPin == GPIO_Pin_9
      && initParams->rxPort == GPIOA && initParams->rxPin == GPIO_Pin_10) {
    GPIO_setAlternateFunction(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, 0);
  } else {
    assert_param(0);
  }
#else
#  error "No valid chip defined"
#endif

  // GPIO
  GPIO_initParamsInit(&gpio);
  gpio.port = initParams->txPort;
  gpio.pin = initParams->txPin;
  gpio.mode = GPIO_Mode_alternateFunctionOutput;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  gpio.port = initParams->rxPort;
  gpio.pin = initParams->rxPin;
  gpio.mode = GPIO_Mode_alternateFunctionInput;
  GPIO_init(&gpio);

  HAL_USART_init(&initParams->halUsartInitParams);
}

void USART_txString(USART_Instance instance, const char *str) {
  const char *p = str;

  assert_param(str != NULL);

  while (*p) {
    USART_txWaitForComplete(instance);
    USART_tx(instance, *p);
    p++;
  }
}

void USART_txBytes(USART_Instance instance, uint8_t *data, uint32_t offset, uint32_t len) {
  uint32_t end = offset + len;
  uint32_t i = offset;

  assert_param(data != NULL);

  while (i < end) {
    USART_txWaitForComplete(instance);
    USART_tx(instance, data[i]);
    i++;
  }
}
