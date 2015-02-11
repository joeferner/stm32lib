
#ifndef _STM32LIB_HAL_GPIO_H_
#define _STM32LIB_HAL_GPIO_H_

#include "base.h"

typedef GPIO_TypeDef *GPIO_Port;
#define IS_GPIO_PORT(p) ( \
    (((GPIO_TypeDef*)p) == GPIOA) || \
    (((GPIO_TypeDef*)p) == GPIOB) || \
    (((GPIO_TypeDef*)p) == GPIOC) || \
    (((GPIO_TypeDef*)p) == GPIOD) || \
    (((GPIO_TypeDef*)p) == GPIOE) || \
    (((GPIO_TypeDef*)p) == GPIOF)    \
  )

typedef enum {
  GPIO_Pin_0 =   0b0000000000000001,
  GPIO_Pin_1 =   0b0000000000000010,
  GPIO_Pin_2 =   0b0000000000000100,
  GPIO_Pin_3 =   0b0000000000001000,
  GPIO_Pin_4 =   0b0000000000010000,
  GPIO_Pin_5 =   0b0000000000100000,
  GPIO_Pin_6 =   0b0000000001000000,
  GPIO_Pin_7 =   0b0000000010000000,
  GPIO_Pin_8 =   0b0000000100000000,
  GPIO_Pin_9 =   0b0000001000000000,
  GPIO_Pin_10 =  0b0000010000000000,
  GPIO_Pin_11 =  0b0000100000000000,
  GPIO_Pin_12 =  0b0001000000000000,
  GPIO_Pin_13 =  0b0010000000000000,
  GPIO_Pin_14 =  0b0100000000000000,
  GPIO_Pin_15 =  0b1000000000000000,
  GPIO_Pin_All = 0b1111111111111111
} GPIO_Pin;

typedef enum {
  GPIO_Mode_input = 0b00,
  GPIO_Mode_output = 0b01,
  GPIO_Mode_alternateFunction = 0b10,
  GPIO_Mode_analog = 0b11,
} GPIO_Mode;
#define GPIO_Mode_default GPIO_Mode_input
#define IS_GPIO_MODE(v) ( \
    ((v) == GPIO_Mode_input) || \
    ((v) == GPIO_Mode_output) || \
    ((v) == GPIO_Mode_alternateFunction) || \
    ((v) == GPIO_Mode_analog) \
  )

typedef enum {
  GPIO_OutputType_pushPull = 0b0,
  GPIO_OutputType_openDrain = 0b1
} GPIO_OutputType;
#define GPIO_OutputType_default GPIO_OutputType_pushPull
#define IS_GPIO_OUTPUT_TYPE(v) ( \
    ((v) == GPIO_OutputType_pushPull) || \
    ((v) == GPIO_OutputType_openDrain) \
  )

typedef enum {
  GPIO_Speed_low = 0b00,
  GPIO_Speed_medium = 0b01,
  GPIO_Speed_high = 0b11
} GPIO_Speed;
#define IS_GPIO_SPEED(v) ( \
    ((v) == GPIO_Speed_low) || \
    ((v) == GPIO_Speed_medium) || \
    ((v) == GPIO_Speed_high) \
  )

typedef enum {
  GPIO_PullUpDown_no = 0b00,
  GPIO_PullUpDown_pullUp = 0b01,
  GPIO_PullUpDown_pullDown = 0b10
} GPIO_PullUpDown;
#define IS_GPIO_PULL_UP_DOWN(v) ( \
    ((v) == GPIO_PullUpDown_no) || \
    ((v) == GPIO_PullUpDown_pullUp) || \
    ((v) == GPIO_PullUpDown_pullDown) \
  )

typedef enum {
  GPIO_Bit_reset = 0,
  GPIO_Bit_set = 1
} GPIO_BitAction;
#define IS_GPIO_BIT_ACTION(v) ( ((v) == GPIO_Bit_reset) || ((v) == GPIO_Bit_set) )

typedef struct {
  GPIO_Port port;
  GPIO_Pin pin;
  GPIO_Mode mode;
  GPIO_OutputType outputType;
  GPIO_Speed speed;
  GPIO_PullUpDown pullUpDown;
} GPIO_InitParams;

void GPIO_initParamsInit(GPIO_InitParams *initParams);
void GPIO_init(GPIO_InitParams *initParams);
void GPIO_setBits(GPIO_Port port, GPIO_Pin pin);
void GPIO_resetBits(GPIO_Port port, GPIO_Pin pin);
void GPIO_writeBits(GPIO_Port port, GPIO_Pin pin, GPIO_BitAction bitAction);
GPIO_BitAction GPIO_readInputBit(GPIO_Port port, GPIO_Pin pin);

#endif
