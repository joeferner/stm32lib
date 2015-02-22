
#include "platform_config.h"
#include <stdio.h>

bool gpioInterruptOccured;
EXTI_Line gpioInterruptLine;
uint32_t lastTime;

static void setup();
static void gpio_setup();
static void loop();

int main(void) {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

static void setup() {
  debug_setup();
  time_setup();
  gpio_setup();
  printf("setup complete!\n");
}

static void gpio_setup() {
  GPIO_InitParams gpio;
  EXTI_InitParams exti;
  NVIC_InitParams nvic;

  RCC_peripheralClockEnableForPort(BUTTON_PORT);
  GPIO_initParamsInit(&gpio);
  gpio.port = BUTTON_PORT;
  gpio.pin = BUTTON_PIN;
  gpio.mode = GPIO_Mode_input;
  GPIO_init(&gpio);

  gpioInterruptOccured = false;
  gpioInterruptLine = EXTI_getLineForGpio(BUTTON_PORT, BUTTON_PIN);

  GPIO_EXTILineConfig(BUTTON_PORT, BUTTON_PIN);
  EXTI_initParamsInit(&exti);
  exti.line = gpioInterruptLine;
  EXTI_enable(&exti);

  nvic.channel = EXTI_getIRQForGpio(BUTTON_PORT, BUTTON_PIN);
  nvic.priority = 0x03;
  NVIC_enable(&nvic);

  printf("gpio setup complete!\n");
}

void onExti() {
  gpioInterruptOccured = true;
}

#include <stm32lib/hal/chip/chip.h>


static void loop() {
  GPIO_TypeDef *gpio = GPIOC;
  RCC_TypeDef *rcc = RCC;
  SYSCFG_TypeDef *syscfg = SYSCFG;
  EXTI_TypeDef *exti = EXTI;
  NVIC_Type *nvic = NVIC;
  uint32_t primask = __get_PRIMASK();

  if (gpioInterruptOccured) {
    printf("interrupt!\n");
    gpioInterruptOccured = false;
  }
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}
