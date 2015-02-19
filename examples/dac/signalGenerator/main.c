
#include "platform_config.h"
#include <stdio.h>
#include <math.h>

#define PATTERN_SIZE 100
#define PI           3.14159265

DAC_Channel dacChannel;
uint32_t patternOffset;
uint16_t pattern[PATTERN_SIZE];

static void setup();
static void setup_dac();
static void loop();
static void setup_pattern();

int main(void) {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

static void setup() {
  debug_setup();
  setup_dac();
  setup_pattern();
  printf("setup complete!\n");
}

static void setup_dac() {
  DAC_InitParams dac;

  dacChannel = DAC_getChannelFromPortAndPin(DAC_PORT, DAC_PIN);

  DAC_initParamsInit(&dac);
  dac.halDacInitParams.channel = dacChannel;
  dac.port = DAC_PORT;
  dac.pin = DAC_PIN;
  DAC_init(&dac);

  DAC_enable(dac.halDacInitParams.channel);
  printf("DAC setup complete!\n");
}

static void setup_pattern() {
  uint32_t i;
  double x, d;

  printf("generating pattern\n");
  patternOffset = 0;
  for (i = 0; i < PATTERN_SIZE; i++) {
    x = ((double)i / (double)PATTERN_SIZE) * (2.0 * PI);
    d = sin(x);
    pattern[i] = clamp((d * 2048) + 2048, 0, 4096 - 1);
  }
  printf("generating pattern complete!\n");
}

static void loop() {  
  DAC_set(dacChannel, DAC_Alignment_12bitRight, pattern[patternOffset++]);
  if (patternOffset >= PATTERN_SIZE) {
    patternOffset = 0;
  }
  sleep_us(100);
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}

