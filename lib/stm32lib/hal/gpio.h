
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
  GPIO_Pin_15 =  0b1000000000000000
} GPIO_Pin;
#define IS_GPIO_PIN(v) ( \
    ((v) == GPIO_Pin_0) \
    || ((v) == GPIO_Pin_1) \
    || ((v) == GPIO_Pin_2) \
    || ((v) == GPIO_Pin_3) \
    || ((v) == GPIO_Pin_4) \
    || ((v) == GPIO_Pin_5) \
    || ((v) == GPIO_Pin_6) \
    || ((v) == GPIO_Pin_7) \
    || ((v) == GPIO_Pin_8) \
    || ((v) == GPIO_Pin_9) \
    || ((v) == GPIO_Pin_10) \
    || ((v) == GPIO_Pin_11) \
    || ((v) == GPIO_Pin_12) \
    || ((v) == GPIO_Pin_13) \
    || ((v) == GPIO_Pin_14) \
    || ((v) == GPIO_Pin_15) \
  )

#ifdef STM32F0XX
typedef enum {
  GPIO_Mode_input = 0b00,
  GPIO_Mode_output = 0b01,
  GPIO_Mode_alternateFunctionInput = 0b10,
  GPIO_Mode_alternateFunctionOutput = 0b10,
  GPIO_Mode_analog = 0b11
} GPIO_Mode;
#elif defined(STM32F10X)
typedef enum {
  GPIO_Mode_input = 1,
  GPIO_Mode_output = 2,
  GPIO_Mode_alternateFunctionInput = 3,
  GPIO_Mode_alternateFunctionOutput = 4,
  GPIO_Mode_analog = 5
} GPIO_Mode;
#else
#  error "No valid chip defined"
#endif
#define GPIO_Mode_default GPIO_Mode_input
#define IS_GPIO_MODE(v) ( \
    ((v) == GPIO_Mode_input) \
    || ((v) == GPIO_Mode_output) \
    || ((v) == GPIO_Mode_alternateFunctionInput) \
    || ((v) == GPIO_Mode_alternateFunctionOutput) \
    || ((v) == GPIO_Mode_analog) \
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

#ifdef STM32F0XX
typedef enum {
} SWJTAG_State;
#elif defined (STM32F10X)
typedef enum {
  SWJTAG_State_swAndJtag = 0x00000000,
  SWJTAG_State_swAndJtagNoNJTRST = 0x01000000,
  SWJTAG_State_sw = 0x02000000,
  SWJTAG_State_off = 0x04000000
} SWJTAG_State;
#define SWJTAG_State_mask 0x07000000
#define IS_SWJTAG_STATE(v) ( \
    ((v) == SWJTAG_State_swAndJtag) || \
    ((v) == SWJTAG_State_swAndJtagNoNJTRST) || \
    ((v) == SWJTAG_State_sw) || \
    ((v) == SWJTAG_State_off) \
  )
#else
#  error "No valid chip defined"
#endif

void GPIO_initParamsInit(GPIO_InitParams *initParams);
void GPIO_init(GPIO_InitParams *initParams);
void GPIO_setBits(GPIO_Port port, GPIO_Pin pin);
void GPIO_resetBits(GPIO_Port port, GPIO_Pin pin);
void GPIO_writeBits(GPIO_Port port, GPIO_Pin pin, GPIO_BitAction bitAction);
GPIO_BitAction GPIO_readInputBit(GPIO_Port port, GPIO_Pin pin);
void GPIO_setAlternateFunction(GPIO_Port port, GPIO_Pin pin, uint8_t af);
void GPIO_EXTILineConfig(GPIO_Port port, GPIO_Pin pin);

void SWJTAG_setup(SWJTAG_State state);

#endif
