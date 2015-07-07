
#ifndef _STM32LIB_HAL_ADC_H_
#define _STM32LIB_HAL_ADC_H_

#include "base.h"

typedef enum {
  ADC_Channel_4 = 0x04
} ADC_Channel;
#define ADC_Channel_notSet 0xff
#define IS_ADC_CHANNEL(ch) ( \
  ((ch) == ADC_Channel_4) \
)

typedef enum {
  ADC_ContinuousConversion_single = 0x00000000,
  ADC_ContinuousConversion_continuous = 0x00000002,
} ADC_ContinuousConversion;
#define ADC_ContinuousConversion_mask 0x00000002

typedef enum {
  ADC_SampleTime_1_5 = 0b000,
  ADC_SampleTime_7_5 = 0b001,
  ADC_SampleTime_13_5 = 0b010,
  ADC_SampleTime_28_5 = 0b011,
  ADC_SampleTime_41_5 = 0b100,
  ADC_SampleTime_55_5 = 0b101,
  ADC_SampleTime_71_5 = 0b110,
  ADC_SampleTime_239_5 = 0b111
} ADC_SampleTime;

typedef struct {
  ADC_TypeDef* instance;
  ADC_Channel channel;
  ADC_ContinuousConversion continuousConversion;
  ADC_SampleTime sampleTime;
} HAL_ADC_InitParams;

void HAL_ADC_initParamsInit(HAL_ADC_InitParams *initParams);
void HAL_ADC_init(HAL_ADC_InitParams *initParams);
void ADC_enable(HAL_ADC_InitParams* instance);
void ADC_disable(HAL_ADC_InitParams* instance);
void ADC_setState(HAL_ADC_InitParams* instance, FunctionalState state);
uint16_t ADC_read(HAL_ADC_InitParams* instance);

#endif
