
#include "platform_config.h"
#include <stdio.h>

CC3000 cc3000;

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
  debug_setup();
  
  cc3000.patchReq = false;
  cc3000.connectPolicy = CC3000_ConnectPolicy_openAP | CC3000_ConnectPolicy_fast | CC3000_ConnectPolicy_profiles;
  cc3000.deviceName = "cc3000test";
  cc3000.spi = CC3000_SPI;
  cc3000.csPort = CC3000_CS_PORT;
  cc3000.csPin = CC3000_CS_PIN;
  cc3000.irqPort = CC3000_IRQ_PORT;
  cc3000.irqPin = CC3000_IRQ_PIN;
  cc3000.enPort = CC3000_EN_PORT;
  cc3000.enPin = CC3000_EN_PIN;
  cc3000_setupGpio(&cc3000);
  cc3000_setup(&cc3000);
  printf("setup complete!\n");
}

static void loop() {

}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}
