
#include "spi.h"
#include "rcc.h"

void HAL_SPI_initParamsInit(HAL_SPI_InitParams *initParams) {
  initParams->instance = NULL;
  initParams->direction = SPI_Direction_2LinesFullDuplex;
  initParams->mode = SPI_Mode_master;
  initParams->dataSize = SPI_DataSize_8b;
  initParams->nss = SPI_NSS_soft;
  initParams->baudRatePrescaler = SPI_BaudRatePrescaler_256;
  initParams->firstBit = SPI_FirstBit_msb;
  initParams->cpol = SPI_CPOL_low;
  initParams->cpha = SPI_CPHA_1Edge;
}

void HAL_SPI_init(HAL_SPI_InitParams *initParams) {
  uint16_t tmp;

  assert_param(IS_SPI_INSTANCE(initParams->instance));
  assert_param(IS_SPI_DIRECTION(initParams->direction));
  assert_param(IS_SPI_MODE(initParams->mode));
  assert_param(IS_SPI_FIRST_BIT(initParams->firstBit));
  assert_param(IS_SPI_DATASIZE(initParams->dataSize));
  assert_param(IS_SPI_CPOL(initParams->cpol));
  assert_param(IS_SPI_CPHA(initParams->cpha));
  assert_param(IS_SPI_NSS(initParams->nss));
  assert_param(IS_SPI_BAUD_RATE_PRESCALER(initParams->baudRatePrescaler));

  if (initParams->instance == SPI1) {
    RCC_peripheralClockEnable(RCC_Peripheral_SPI1);
  } else {
    assert_param(0);
  }

  // CR1
  tmp = initParams->instance->CR1;

  tmp &= ~SPI_Direction_mask;
  tmp |= initParams->direction;

  tmp &= ~SPI_Mode_mask;
  tmp |= initParams->mode;

  tmp &= ~SPI_NSS_mask;
  tmp |= initParams->nss;

  tmp &= ~SPI_FirstBit_mask;
  tmp |= initParams->firstBit;

  tmp &= ~SPI_BaudRatePrescaler_mask;
  tmp |= initParams->baudRatePrescaler;

  tmp &= ~SPI_CPOL_mask;
  tmp |= initParams->cpol;

  tmp &= ~SPI_CPHA_mask;
  tmp |= initParams->cpha;

  initParams->instance->CR1 = tmp;

  // CR2
  tmp = initParams->instance->CR2;

  tmp &= ~SPI_DataSize_mask;
  tmp |= initParams->dataSize;

  if (IS_SPI_DATASIZE_8BIT_OR_LESS(initParams->dataSize)) {
    tmp |= SPI_CR2_FRXTH;
  } else {
    tmp &= ~SPI_CR2_FRXTH;
  }

  initParams->instance->CR2 = tmp;
}

void SPI_enable(SPI_Instance instance) {
  assert_param(IS_SPI_INSTANCE(instance));
  SPI_setState(instance, ENABLE);
}

void SPI_disable(SPI_Instance instance) {
  assert_param(IS_SPI_INSTANCE(instance));
  SPI_setState(instance, DISABLE);
}

void SPI_setState(SPI_Instance instance, FunctionalState state) {
  assert_param(IS_SPI_INSTANCE(instance));
  if (state != DISABLE) {
    instance->CR1 |= SPI_CR1_SPE;
  } else {
    instance->CR1 &= ~SPI_CR1_SPE;
  }
}

void SPI_sendData8(SPI_Instance instance, uint8_t d) {
  assert_param(IS_SPI_INSTANCE(instance));
  *((volatile uint8_t *)&instance->DR) = d;
}

uint8_t SPI_receiveData8(SPI_Instance instance) {
  assert_param(IS_SPI_INSTANCE(instance));
  return *((volatile uint8_t *)&instance->DR);
}

void SPI_sendData16(SPI_Instance instance, uint16_t d) {
  assert_param(IS_SPI_INSTANCE(instance));
  instance->DR = d;
}

uint16_t SPI_receiveData16(SPI_Instance instance) {
  assert_param(IS_SPI_INSTANCE(instance));
  return instance->DR;
}

FlagStatus SPI_getFlagStatus(SPI_Instance instance, SPI_Flag flag) {
  assert_param(IS_SPI_INSTANCE(instance));
  assert_param(IS_SPI_FLAG(flag));
  if ((instance->SR & flag) != (uint16_t)RESET) {
    return SET;
  } else {
    return RESET;
  }
}



