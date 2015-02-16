
#include "platform_config.h"
#include <stdio.h>

static void setup();
static void led_setup();
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
  led_setup();
  printf("setup complete!\n");
}

static void led_setup() {
  GPIO_InitParams gpio;

  RCC_peripheralClockEnableForPort(LED_PORT);

  GPIO_initParamsInit(&gpio);
  gpio.port = LED_PORT;
  gpio.pin = LED_PIN;
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);
}

static void loop() {
  GPIO_setBits(LED_PORT, LED_PIN);
  sleep_ms(1000);
  GPIO_resetBits(LED_PORT, LED_PIN);
  sleep_ms(1000);
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}

