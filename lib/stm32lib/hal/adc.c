
#include "adc.h"
#include <stdio.h>

void HAL_ADC_initParamsInit(HAL_ADC_InitParams *initParams) {
  initParams->instance = ADC1;
  initParams->channel = ADC_Channel_notSet;
  initParams->sampleTime = ADC_SampleTime_239_5;
  initParams->continuousConversion = ADC_ContinuousConversion_continuous;
}

void HAL_ADC_init(HAL_ADC_InitParams *initParams) {
  uint32_t tmp, tmp2;
  assert_param(IS_ADC_CHANNEL(initParams->channel));
  
  tmp = initParams->instance->CR2;
  tmp &= ~ADC_ContinuousConversion_mask;
  tmp |= initParams->continuousConversion;
  initParams->instance->CR2 = tmp;
  
  tmp2 = initParams->instance->SMPR2;
  if(initParams->channel == ADC_Channel_4) {
    tmp2 &= ~(0b111 << (3 * 4));
    tmp2 |= (initParams->sampleTime & 0b111) << (3 * 4);
  } else {
    assert_param(0);
  }
  initParams->instance->SMPR2 = tmp2;
  
  // TODO make this generic
  initParams->instance->SQR1 = 0x00000000;
  initParams->instance->SQR2 = 0x00000000;
  initParams->instance->SQR3 = 0x00000004;
}

void ADC_enable(HAL_ADC_InitParams* initParams) {
  ADC_setState(initParams, ENABLE);
}

void ADC_disable(HAL_ADC_InitParams* initParams) {
  ADC_setState(initParams, DISABLE);
}

void ADC_setState(HAL_ADC_InitParams* initParams, FunctionalState state) {
  assert_param(IS_FUNCTIONAL_STATE(state));

  if (state != DISABLE) {
    initParams->instance->CR2 |= ADC_CR2_ADON;
  } else {
    initParams->instance->CR2 &= ~ADC_CR2_ADON;
  }
}

uint16_t ADC_read(HAL_ADC_InitParams* initParams) {
  while(!(initParams->instance->SR & ADC_SR_EOC));
  return initParams->instance->DR & 0xffff;
}
