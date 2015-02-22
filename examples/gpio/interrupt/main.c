
#include "platform_config.h"
#include <stdio.h>

volatile bool gpioInterruptOccured;
volatile uint32_t gpioInterruptTime;
volatile EXTI_Line gpioInterruptLine;
volatile IRQn_Type gpioIrq;

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

  RCC_peripheralClockEnableForPort(BUTTON_PORT);
  GPIO_initParamsInit(&gpio);
  gpio.port = BUTTON_PORT;
  gpio.pin = BUTTON_PIN;
  gpio.mode = GPIO_Mode_input;
  GPIO_init(&gpio);

  gpioInterruptOccured = false;
  gpioInterruptLine = EXTI_getLineForGpio(BUTTON_PORT, BUTTON_PIN);
  gpioIrq = EXTI_getIRQForGpio(BUTTON_PORT, BUTTON_PIN);

  EXTI_initParamsInit(&exti);
  exti.line = gpioInterruptLine;
  exti.trigger = EXTI_Trigger_falling;
  EXTI_enable(&exti);
  GPIO_EXTILineConfig(BUTTON_PORT, BUTTON_PIN); // must be called after EXTI_enable
  NVIC_EnableIRQ(gpioIrq);

  printf("gpio setup complete!\n");
}

void onExti() {
  gpioInterruptOccured = true;
  gpioInterruptTime = time_ms();
}

static void loop() {
  if (gpioInterruptOccured) {
    printf("interrupt at %lu!\n", gpioInterruptTime);
    gpioInterruptOccured = false;
  }
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}
